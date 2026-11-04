#!/usr/bin/env python3
# Descent 3
# Copyright (C) 2024 Parallax Software
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""Records where vendored third-party code came from and checks it.

A component is a directory that holds an UPSTREAM.toml manifest, either
third_party/<name>/ or a top-level <name>/ directory. The manifest names the
upstream git repository, the pinned commit, the upstream files that were
copied and where they live locally, and an ordered list of patches in the
component's patches/ directory. third_party/README.md describes the fields.

Commands:
  check [NAME...]       Rebuild each component from upstream plus patches in a
                        temporary directory and compare the result with the
                        files in the working tree. Without names, all
                        components are checked and THIRD_PARTY.md is compared
                        with the manifests too. Exits 1 on any mismatch.
  update NAME [REF]     Fetch upstream at REF (a full commit hash, a tag or a
                        branch; the pinned commit if omitted), apply the
                        patches and write the result into the working tree.
                        With REF, the manifest's commit is updated as well.
                        Nothing is written if a patch does not apply.
  diff NAME -m MESSAGE  Record the differences between the working tree and
                        upstream plus the existing patches as a new patch file
                        and add it to the manifest.
  docs [--check]        Regenerate the per-component sections of
                        THIRD_PARTY.md, or with --check only compare them.

Comparison: files are compared in the form git stores them for their local
path. Both the expected content and the working tree file are passed through
`git hash-object --path=<local path>`, which applies the line ending
conversion set by .gitattributes (and core.autocrlf), so an upstream file with
CRLF line endings matches a local copy that git normalizes to LF. Any other
difference, whitespace included, is a mismatch.

Upstream content is treated as data. It is fetched into a new temporary git
repository with the user's and the system's git configuration disabled, read
with `git cat-file`, and never checked out, built or run.

Requires Python 3.11 or later (for tomllib) and git 2.32 or later.
"""

import argparse
import difflib
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path, PurePosixPath

try:
    import tomllib
except ModuleNotFoundError:
    sys.exit("vendor.py: needs Python 3.11 or later (the tomllib module is missing)")

REPO = Path(__file__).resolve().parent.parent
MANIFEST = "UPSTREAM.toml"
PATCH_DIR = "patches"
DOC_FILE = "THIRD_PARTY.md"
DOC_BEGIN = "<!-- BEGIN sections generated from UPSTREAM.toml files by tools/vendor.py docs -->"
DOC_END = "<!-- END sections generated from UPSTREAM.toml files -->"

# How many lines of a content diff to show for a modified file.
DIFF_LINES = 40

TOP_KEYS = {"url", "commit", "version", "fork", "local_files", "patches",
            "license_file", "files", "version_check", "stamps", "doc"}
DOC_KEYS = {"title", "summary", "notes", "license", "license_text"}
SHA_RE = re.compile(r"^[0-9a-f]{40}$")


class VendorError(Exception):
    pass


# ---------------------------------------------------------------------------
# git helpers

def isolated_env():
    """Environment for git commands that touch upstream data or temp dirs.

    The user's and the system's git configuration are ignored, so settings
    such as url rewrites, autocrlf, diff drivers or credential helpers cannot
    change what is fetched, applied or diffed.
    """
    env = dict(os.environ)
    env["GIT_CONFIG_NOSYSTEM"] = "1"
    env["GIT_CONFIG_GLOBAL"] = os.devnull
    env["GIT_TERMINAL_PROMPT"] = "0"
    env["GIT_ASKPASS"] = ""
    env["SSH_ASKPASS"] = ""
    for key in ("GIT_DIR", "GIT_WORK_TREE", "GIT_INDEX_FILE", "GIT_OBJECT_DIRECTORY",
                "GIT_ALTERNATE_OBJECT_DIRECTORIES", "GIT_CEILING_DIRECTORIES"):
        env.pop(key, None)
    return env


def run_git(args, cwd, env=None, data=None, ok_codes=(0,)):
    proc = subprocess.run(["git", *args], cwd=cwd, env=env, input=data,
                          stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if proc.returncode not in ok_codes:
        msg = proc.stderr.decode("utf-8", "replace").strip()
        raise VendorError(f"git {' '.join(args)} failed (exit {proc.returncode}): {msg}")
    return proc


class StoredForm:
    """Converts content to the bytes git would store for a path in this repo.

    The objects are written to a temporary object directory, not to the
    repository's own one.
    """

    def __init__(self, tmp):
        self.objdir = Path(tmp) / "objects"
        self.objdir.mkdir()
        # The repository's own objects stay readable (git may look at the
        # index entry's blob when deciding on line ending conversion).
        repo_objects = run_git(["rev-parse", "--path-format=absolute", "--git-path", "objects"],
                               REPO).stdout.decode().strip()
        self.env = dict(os.environ)
        self.env["GIT_OBJECT_DIRECTORY"] = str(self.objdir)
        self.env["GIT_ALTERNATE_OBJECT_DIRECTORIES"] = repo_objects

    def convert(self, repo_path, data):
        oid = run_git(["-c", "core.safecrlf=false", "hash-object", "-w", "--stdin",
                       f"--path={repo_path}"], REPO, self.env, data).stdout.strip()
        return run_git(["cat-file", "blob", oid.decode()], REPO, self.env).stdout


class Upstream:
    """A fresh bare repository holding one fetched upstream commit."""

    def __init__(self, url, tmp):
        self.url = url
        self.dir = Path(tmp) / "upstream.git"
        self.env = isolated_env()
        run_git(["init", "-q", "--bare", str(self.dir)], tmp, self.env)
        run_git(["remote", "add", "origin", url], self.dir, self.env)

    def fetch(self, ref):
        # Blobs are fetched on demand by cat-file, so only the listed files are
        # downloaded.
        try:
            run_git(["fetch", "-q", "--depth=1", "--filter=blob:none", "--no-tags",
                     "origin", ref], self.dir, self.env)
        except VendorError as e:
            raise VendorError(f"cannot fetch {ref} from {self.url}: {e}") from e
        commit = run_git(["rev-parse", "--verify", "FETCH_HEAD^{commit}"],
                         self.dir, self.env).stdout.decode().strip()
        self.commit = commit
        return commit

    def read(self, path):
        """Returns the blob at path, or None if the path is not a regular file."""
        out = run_git(["ls-tree", "-z", self.commit, "--", path], self.dir, self.env).stdout
        for entry in out.split(b"\0"):
            if not entry:
                continue
            meta, name = entry.split(b"\t", 1)
            mode, kind, oid = meta.split(b" ")
            if name.decode() == path and kind == b"blob" and mode in (b"100644", b"100755"):
                return run_git(["cat-file", "blob", oid.decode()], self.dir, self.env).stdout
        return None


# ---------------------------------------------------------------------------
# manifests

def check_rel_path(value, what):
    if not isinstance(value, str) or not value:
        raise VendorError(f"{what}: expected a non-empty string, got {value!r}")
    p = PurePosixPath(value)
    if "\\" in value or p.is_absolute() or ".." in p.parts or "." in p.parts:
        raise VendorError(f"{what}: {value!r} must be a relative path without '.' or '..'")
    return value


class Component:
    def __init__(self, manifest_path):
        self.manifest = manifest_path
        self.dir = manifest_path.parent
        self.name = self.dir.name
        self.rel_dir = self.dir.relative_to(REPO).as_posix()
        try:
            with open(manifest_path, "rb") as f:
                data = tomllib.load(f)
        except tomllib.TOMLDecodeError as e:
            raise VendorError(f"{self.rel_manifest()}: {e}") from e
        self.parse(data)

    def rel_manifest(self):
        return f"{self.rel_dir}/{MANIFEST}"

    def parse(self, d):
        where = self.rel_manifest()
        unknown = set(d) - TOP_KEYS
        if unknown:
            raise VendorError(f"{where}: unknown keys: {', '.join(sorted(unknown))}")

        self.url = d.get("url")
        if not isinstance(self.url, str) or not self.url.startswith("https://"):
            raise VendorError(f"{where}: url must be an https:// URL, got {self.url!r}")
        self.commit = d.get("commit")
        if not isinstance(self.commit, str) or not SHA_RE.match(self.commit):
            raise VendorError(f"{where}: commit must be a full 40-character hash, got {self.commit!r}")
        self.version = d.get("version")
        if self.version is not None and not isinstance(self.version, str):
            raise VendorError(f"{where}: version must be a string")
        self.fork = d.get("fork", False)
        if not isinstance(self.fork, bool):
            raise VendorError(f"{where}: fork must be true or false")

        files = d.get("files")
        if not isinstance(files, dict) or not files:
            raise VendorError(f"{where}: [files] must map upstream paths to local paths")
        self.files = {}
        for up, local in files.items():
            check_rel_path(up, f"{where}: upstream path")
            check_rel_path(local, f"{where}: local path for {up}")
            self.files[up] = local
        locals_ = list(self.files.values())
        if len(set(locals_)) != len(locals_):
            raise VendorError(f"{where}: two upstream files map to the same local path")

        self.local_files = d.get("local_files", [])
        if not isinstance(self.local_files, list):
            raise VendorError(f"{where}: local_files must be a list")
        for p in self.local_files:
            check_rel_path(p, f"{where}: local_files entry")
            if p in self.files.values():
                raise VendorError(f"{where}: {p} is in both [files] and local_files")

        for p in locals_ + self.local_files:
            if p == MANIFEST or PurePosixPath(p).parts[0] == PATCH_DIR:
                raise VendorError(f"{where}: {p} collides with the manifest or {PATCH_DIR}/")

        if self.fork:
            if "patches" in d:
                raise VendorError(f"{where}: a fork has no patches; remove the patches key")
            self.patches = []
        else:
            if "patches" not in d:
                raise VendorError(f"{where}: patches is required (use patches = [] for none)")
            self.patches = d["patches"]
            if not isinstance(self.patches, list):
                raise VendorError(f"{where}: patches must be a list")
            for p in self.patches:
                check_rel_path(p, f"{where}: patch")
                if "/" in p or not p.endswith(".patch"):
                    raise VendorError(f"{where}: patch {p!r} must be a .patch file name in {PATCH_DIR}/")
            if len(set(self.patches)) != len(self.patches):
                raise VendorError(f"{where}: a patch is listed twice")

        self.license_file = d.get("license_file")
        if self.license_file is not None:
            check_rel_path(self.license_file, f"{where}: license_file")
            if self.license_file not in locals_ + self.local_files:
                raise VendorError(f"{where}: license_file {self.license_file} is not one of the component's files")

        self.version_check = d.get("version_check")
        if self.version_check is not None:
            vc = self.version_check
            if (not isinstance(vc, dict) or set(vc) != {"upstream_file", "regex"}
                    or not isinstance(vc["regex"], str)):
                raise VendorError(f"{where}: [version_check] needs exactly upstream_file and regex")
            check_rel_path(vc["upstream_file"], f"{where}: version_check.upstream_file")
            try:
                if re.compile(vc["regex"]).groups != 1:
                    raise VendorError(f"{where}: version_check.regex needs exactly one group")
            except re.error as e:
                raise VendorError(f"{where}: version_check.regex: {e}") from e
            if self.version is None:
                raise VendorError(f"{where}: [version_check] needs a version")

        self.stamps = d.get("stamps", [])
        if not isinstance(self.stamps, list):
            raise VendorError(f"{where}: stamps must be an array of tables")
        for s in self.stamps:
            if not isinstance(s, dict) or set(s) != {"file", "line"}:
                raise VendorError(f"{where}: each [[stamps]] entry needs exactly file and line")
            check_rel_path(s["file"], f"{where}: stamps.file")
            if "{version}" in s["line"] and self.version is None:
                raise VendorError(f"{where}: a stamp uses {{version}} but there is no version")
            if "{commit}" in s["line"] and self.commit is None:
                raise VendorError(f"{where}: a stamp uses {{commit}} but there is no commit")

        doc = d.get("doc")
        if not isinstance(doc, dict):
            raise VendorError(f"{where}: [doc] is required")
        unknown = set(doc) - DOC_KEYS
        if unknown:
            raise VendorError(f"{where}: unknown keys in [doc]: {', '.join(sorted(unknown))}")
        for key in ("title", "summary"):
            if not isinstance(doc.get(key), str) or not doc[key].strip():
                raise VendorError(f"{where}: doc.{key} is required")
        self.doc = doc

    def repo_path(self, local):
        return f"{self.rel_dir}/{local}"

    def patch_path(self, patch):
        return self.dir / PATCH_DIR / patch


def find_components():
    found = sorted(REPO.glob(f"third_party/*/{MANIFEST}")) + sorted(REPO.glob(f"*/{MANIFEST}"))
    comps = {}
    for m in found:
        c = Component(m)
        if c.name in comps:
            raise VendorError(f"two components are named {c.name}: "
                              f"{comps[c.name].rel_manifest()} and {c.rel_manifest()}")
        comps[c.name] = c
    return comps


def select(comps, names):
    if not names:
        return list(comps.values())
    missing = [n for n in names if n not in comps]
    if missing:
        raise VendorError(f"no component named {', '.join(missing)} "
                          f"(known: {', '.join(sorted(comps))})")
    return [comps[n] for n in names]


# ---------------------------------------------------------------------------
# rebuilding a component

class Build:
    """Upstream files plus patches, in stored form, keyed by local path."""

    def __init__(self, comp, tmp, ref=None):
        self.comp = comp
        self.tmp = Path(tmp)
        self.stored = StoredForm(tmp)
        self.problems = []
        self.failed_patches = []
        self.files = {}
        self.upstream = Upstream(comp.url, tmp)
        self.commit = self.upstream.fetch(ref or comp.commit)
        if ref is None and self.commit != comp.commit:
            raise VendorError(f"{comp.name}: fetched {self.commit}, expected {comp.commit}")

        self.upstream_version = None
        if comp.version_check:
            vc = comp.version_check
            blob = self.upstream.read(vc["upstream_file"])
            if blob is None:
                self.problems.append(f"version_check: {vc['upstream_file']} does not exist upstream")
            else:
                m = re.search(vc["regex"], blob.decode("utf-8", "replace"), re.MULTILINE)
                if not m:
                    self.problems.append(f"version_check: regex not found in upstream {vc['upstream_file']}")
                else:
                    self.upstream_version = m.group(1)

        for up, local in comp.files.items():
            blob = self.upstream.read(up)
            if blob is None:
                self.problems.append(f"{up} is not a regular file at upstream commit {self.commit}")
                continue
            self.files[local] = blob

        if comp.fork:
            return

        tree = self.tmp / "tree"
        tree.mkdir()
        env = isolated_env()
        run_git(["init", "-q", str(tree)], self.tmp, env)
        for local, blob in self.files.items():
            dest = tree / local
            dest.parent.mkdir(parents=True, exist_ok=True)
            dest.write_bytes(self.stored.convert(comp.repo_path(local), blob))

        for patch in comp.patches:
            path = comp.patch_path(patch)
            if not path.is_file():
                self.problems.append(f"patch {PATCH_DIR}/{patch} is listed in {MANIFEST} but does not exist")
                self.failed_patches.append(patch)
                continue
            proc = subprocess.run(["git", "apply", "-p1", "--whitespace=nowarn", str(path)],
                                  cwd=tree, env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            if proc.returncode != 0:
                msg = proc.stderr.decode("utf-8", "replace").strip()
                self.problems.append(f"patch {PATCH_DIR}/{patch} does not apply:\n" + indent(msg))
                self.failed_patches.append(patch)

        present = set()
        for p in tree.rglob("*"):
            rel = p.relative_to(tree)
            if rel.parts[0] == ".git" or not p.is_file():
                continue
            present.add(rel.as_posix())
        for extra in sorted(present - set(self.files)):
            self.problems.append(f"a patch creates {extra}, which is not in [files]")
        for gone in sorted(set(self.files) - present):
            self.problems.append(f"a patch deletes {gone}; remove it from [files] instead")
        self.files = {local: self.stored.convert(comp.repo_path(local), (tree / local).read_bytes())
                      for local in self.files if local in present}


def indent(text, prefix="    "):
    return "\n".join(prefix + line for line in text.splitlines())


def worktree_files(comp):
    """Files in the component directory: tracked plus untracked, not ignored."""
    out = run_git(["ls-files", "-z", "--cached", "--others", "--exclude-standard",
                   "--", comp.rel_dir], REPO).stdout
    result = set()
    for p in out.split(b"\0"):
        if not p:
            continue
        rel = PurePosixPath(p.decode()).relative_to(comp.rel_dir).as_posix()
        if (comp.dir / rel).is_file():
            result.add(rel)
    return result


def content_diff(expected, actual, name):
    a = expected.decode("utf-8", "replace").splitlines()
    b = actual.decode("utf-8", "replace").splitlines()
    lines = list(difflib.unified_diff(a, b, f"expected/{name}", f"checked-in/{name}", lineterm="", n=1))
    if not lines:
        # Same text lines, so the bytes differ only in line endings or a
        # final newline.
        return "    (the difference is only in line endings or the final newline)"
    shown = lines[:DIFF_LINES]
    if len(lines) > DIFF_LINES:
        shown.append(f"... {len(lines) - DIFF_LINES} more diff lines")
    return indent("\n".join(shown))


def compare(comp, build):
    """Returns a list of problems comparing a build with the working tree."""
    problems = list(build.problems)
    present = worktree_files(comp)

    for local in comp.files.values():
        if local not in present:
            problems.append(f"missing: {comp.repo_path(local)} (listed in [files])")
            continue
        if comp.fork or local not in build.files or build.failed_patches:
            continue
        actual = build.stored.convert(comp.repo_path(local), (comp.dir / local).read_bytes())
        if actual != build.files[local]:
            problems.append(f"modified: {comp.repo_path(local)} differs from upstream plus patches\n"
                            + content_diff(build.files[local], actual, local))

    if build.failed_patches:
        problems.append("file contents were not compared because a patch failed")

    for local in comp.local_files:
        if local not in present:
            problems.append(f"missing: {comp.repo_path(local)} (listed in local_files)")

    known = set(comp.files.values()) | set(comp.local_files) | {MANIFEST}
    known |= {f"{PATCH_DIR}/{p}" for p in comp.patches}
    for extra in sorted(present - known):
        if PurePosixPath(extra).parts[0] == PATCH_DIR:
            problems.append(f"unlisted patch: {comp.repo_path(extra)} is not in the manifest's patches list")
        else:
            problems.append(f"extra file: {comp.repo_path(extra)} is not in [files] or local_files")

    if build.upstream_version is not None and build.upstream_version != comp.version:
        problems.append(f"version: manifest says {comp.version}, upstream "
                        f"{comp.version_check['upstream_file']} says {build.upstream_version}")
    problems.extend(check_stamps(comp))
    return problems


def check_stamps(comp):
    problems = []
    for s in comp.stamps:
        line = s["line"].replace("{version}", comp.version or "").replace("{commit}", comp.commit or "")
        path = REPO / s["file"]
        if not path.is_file():
            problems.append(f"stamp: {s['file']} does not exist")
            continue
        lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
        if line not in lines:
            problems.append(f"stamp: {s['file']} has no line {line!r}")
    return problems


# ---------------------------------------------------------------------------
# THIRD_PARTY.md

def doc_section(comp):
    d = comp.doc
    out = [f"## {d['title']}", "", f"{d['summary'].strip()} {comp.url}", ""]
    link = f"[@{comp.commit[:8]}]({comp.url}/tree/{comp.commit})"
    if comp.version:
        link += f" (version {comp.version})"
    out += [link, ""]
    out += [f"* {comp.repo_path(p)}" for p in comp.files.values()]
    out.append("")
    if comp.fork:
        out += ["These files were imported from upstream and have since been renamed and largely "
                "rewritten, so they are no longer compared with upstream. The commit above records "
                "where they came from.", ""]
        if comp.local_files:
            out += ["Files added in Descent 3:", ""]
            out += [f"* {comp.repo_path(p)}" for p in comp.local_files]
            out.append("")
    if comp.patches:
        out += [f"Changes made in Descent 3 are kept as patches in {comp.rel_dir}/{PATCH_DIR}/:", ""]
        out += [f"* {p}" for p in comp.patches]
        out.append("")
    if d.get("notes"):
        out += [d["notes"].strip(), ""]
    if d.get("license"):
        out += [d["license"].strip(), ""]
    if d.get("license_text"):
        out += ["```", d["license_text"].strip("\n"), "```", ""]
    if comp.license_file:
        also = "license text is also" if d.get("license_text") else "full license text is"
        out += [f"The {also} in {comp.repo_path(comp.license_file)}.", ""]
    return "\n".join(out)


def render_docs(comps, current):
    begin = current.find(DOC_BEGIN)
    end = current.find(DOC_END)
    if begin < 0 or end < 0 or end < begin:
        raise VendorError(f"{DOC_FILE}: the markers {DOC_BEGIN!r} and {DOC_END!r} are missing")
    body = "\n".join(doc_section(c) for c in sorted(comps.values(), key=lambda c: c.name))
    return current[:begin] + DOC_BEGIN + "\n\n" + body + "\n" + current[end:]


def docs_problems(comps):
    if not comps:
        return []
    path = REPO / DOC_FILE
    current = path.read_text(encoding="utf-8")
    try:
        wanted = render_docs(comps, current)
    except VendorError as e:
        return [str(e)]
    if wanted == current:
        return []
    diff = difflib.unified_diff(current.splitlines(), wanted.splitlines(),
                                f"{DOC_FILE} (checked in)", f"{DOC_FILE} (from manifests)",
                                lineterm="", n=1)
    lines = list(diff)
    return [f"{DOC_FILE} does not match the manifests; run tools/vendor.py docs\n"
            + indent("\n".join(lines[:DIFF_LINES]))]


# ---------------------------------------------------------------------------
# manifest editing

def edit_manifest(comp, edit, verify):
    """Applies a text edit to the manifest and confirms it parses as intended."""
    old = comp.manifest.read_text(encoding="utf-8")
    new = edit(old)
    comp.manifest.write_text(new, encoding="utf-8")
    try:
        with open(comp.manifest, "rb") as f:
            data = tomllib.load(f)
        if not verify(data):
            raise VendorError(f"{comp.rel_manifest()}: edit did not produce the expected value")
        Component(comp.manifest)
    except Exception:
        comp.manifest.write_text(old, encoding="utf-8")
        raise


def replace_string_key(text, key, value, where):
    pattern = re.compile(rf'^({re.escape(key)}\s*=\s*)"[^"\n]*"', re.MULTILINE)
    new, n = pattern.subn(lambda m: f'{m.group(1)}"{value}"', text)
    if n != 1:
        raise VendorError(f"{where}: expected one line '{key} = \"...\"', found {n}")
    return new


def set_patches(text, patches, where):
    pattern = re.compile(r"^patches\s*=\s*\[[^\]]*\]", re.MULTILINE)
    if patches:
        value = "patches = [\n" + "".join(f'  "{p}",\n' for p in patches) + "]"
    else:
        value = "patches = []"
    new, n = pattern.subn(lambda m: value, text)
    if n != 1:
        raise VendorError(f"{where}: expected one 'patches = [...]' array, found {n}")
    return new


# ---------------------------------------------------------------------------
# commands

def plural(n, one, many=None):
    return f"{n} {one if n == 1 else (many or one + 's')}"


def cmd_check(args):
    comps = find_components()
    failed = 0
    for comp in select(comps, args.names):
        with tempfile.TemporaryDirectory(prefix=f"vendor-{comp.name}-") as tmp:
            try:
                build = Build(comp, tmp)
                problems = compare(comp, build)
            except VendorError as e:
                problems = [str(e)]
        if problems:
            failed += 1
            print(f"FAIL {comp.name} ({comp.rel_dir}):")
            for p in problems:
                print(indent(p, "  "))
        elif comp.fork:
            print(f"ok   {comp.name}: fork of {comp.url} @ {comp.commit[:8]}; "
                  f"origin and file list confirmed, contents not compared")
        else:
            print(f"ok   {comp.name}: {comp.url} @ {comp.commit[:8]} plus "
                  f"{plural(len(comp.patches), 'patch', 'patches')} reproduces "
                  f"{plural(len(comp.files), 'file')}")
    if not args.names:
        problems = docs_problems(comps)
        if problems:
            failed += 1
            print(f"FAIL {DOC_FILE}:")
            for p in problems:
                print(indent(p, "  "))
        else:
            print(f"ok   {DOC_FILE} matches the manifests")
    if failed:
        print(f"\n{failed} check(s) failed. Vendored files must equal upstream plus the listed "
              f"patches. To record an intended edit as a patch, run: "
              f"tools/vendor.py diff <name> -m <description>")
        return 1
    return 0


def cmd_update(args):
    comps = find_components()
    comp = select(comps, [args.name])[0]
    if comp.fork:
        raise VendorError(f"{comp.name} is a fork and is not rebuilt from upstream; update it by hand")
    with tempfile.TemporaryDirectory(prefix=f"vendor-{comp.name}-") as tmp:
        build = Build(comp, tmp, args.ref)
        if build.problems:
            print(f"{comp.name}: not updated, upstream {build.commit} plus patches did not build:")
            for p in build.problems:
                print(indent(p, "  "))
            if build.failed_patches:
                print("Fix or remove the listed patches in the manifest, then run update again.")
            return 1
        for local, data in build.files.items():
            dest = comp.dir / local
            dest.parent.mkdir(parents=True, exist_ok=True)
            dest.write_bytes(data)
            print(f"wrote {comp.repo_path(local)}")
        new_version = build.upstream_version

    if build.commit != comp.commit:
        edit_manifest(comp, lambda t: replace_string_key(t, "commit", build.commit, comp.rel_manifest()),
                      lambda d: d["commit"] == build.commit)
        print(f"{comp.rel_manifest()}: commit {comp.commit} -> {build.commit}")
    if new_version is not None and new_version != comp.version:
        edit_manifest(comp, lambda t: replace_string_key(t, "version", new_version, comp.rel_manifest()),
                      lambda d: d["version"] == new_version)
        print(f"{comp.rel_manifest()}: version {comp.version} -> {new_version}")

    # Read the result back.
    comps = find_components()
    args.names = [comp.name]
    print("checking the result:")
    rc = cmd_check(args)
    if rc == 0 and build.commit != comp.commit:
        print(f"Run tools/vendor.py docs to update {DOC_FILE}.")
    return rc


def slug(text):
    s = re.sub(r"[^a-z0-9]+", "-", text.lower()).strip("-")
    return s[:50].rstrip("-") or "change"


def cmd_diff(args):
    comps = find_components()
    comp = select(comps, [args.name])[0]
    if comp.fork:
        raise VendorError(f"{comp.name} is a fork; its local changes are not tracked as patches")
    with tempfile.TemporaryDirectory(prefix=f"vendor-{comp.name}-") as tmp:
        build = Build(comp, tmp)
        if build.problems:
            print(f"{comp.name}: upstream plus the existing patches does not build:")
            for p in build.problems:
                print(indent(p, "  "))
            return 1
        tmp = Path(tmp)
        a, b = tmp / "a", tmp / "b"
        changed = []
        for local, expected in build.files.items():
            src = comp.dir / local
            if not src.is_file():
                raise VendorError(f"{comp.repo_path(local)} is missing; remove it from [files] instead")
            actual = build.stored.convert(comp.repo_path(local), src.read_bytes())
            if actual != expected:
                changed.append(local)
                for root, data in ((a, expected), (b, actual)):
                    dest = root / local
                    dest.parent.mkdir(parents=True, exist_ok=True)
                    dest.write_bytes(data)
        if not changed:
            print(f"{comp.name}: no local changes; nothing recorded")
            return 0
        proc = run_git(["diff", "--no-index", "--no-prefix", "--no-color", "--no-ext-diff",
                        "--no-renames", "--", "a", "b"], tmp, isolated_env(), ok_codes=(1,))
        body = proc.stdout

    if not args.message.strip():
        raise VendorError("the patch description (-m) is empty")
    numbers = [int(m.group(1)) for p in comp.patches if (m := re.match(r"(\d+)-", p))]
    name = f"{max(numbers, default=0) + 1:04d}-{slug(args.message.strip().splitlines()[0])}.patch"
    path = comp.patch_path(name)
    if path.exists():
        raise VendorError(f"{path.relative_to(REPO)} already exists")
    header = args.message.strip() + "\n\n"
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(header.encode("utf-8") + body)
    new_list = comp.patches + [name]
    try:
        edit_manifest(comp, lambda t: set_patches(t, new_list, comp.rel_manifest()),
                      lambda d: d["patches"] == new_list)
    except Exception:
        path.unlink()
        raise

    # Read back: upstream plus all patches must now give the working tree.
    comp = find_components()[comp.name]
    with tempfile.TemporaryDirectory(prefix=f"vendor-{comp.name}-") as tmp2:
        problems = compare(comp, Build(comp, tmp2))
    if problems:
        edit_manifest(comp, lambda t: set_patches(t, new_list[:-1], comp.rel_manifest()),
                      lambda d: d["patches"] == new_list[:-1])
        path.unlink()
        print(f"{comp.name}: the new patch did not reproduce the working tree, so it was removed:")
        for p in problems:
            print(indent(p, "  "))
        return 1
    print(f"wrote {path.relative_to(REPO).as_posix()} ({', '.join(changed)})")
    print(f"added it to {comp.rel_manifest()}; run tools/vendor.py docs to update {DOC_FILE}")
    return 0


def cmd_docs(args):
    comps = find_components()
    if args.check:
        problems = docs_problems(comps)
        for p in problems:
            print(p)
        if not problems:
            print(f"{DOC_FILE} matches the manifests")
        return 1 if problems else 0
    path = REPO / DOC_FILE
    current = path.read_text(encoding="utf-8")
    wanted = render_docs(comps, current)
    if wanted == current:
        print(f"{DOC_FILE} is up to date")
        return 0
    path.write_text(wanted, encoding="utf-8")
    if docs_problems(find_components()):
        raise VendorError(f"{DOC_FILE} still differs from the manifests after writing it")
    print(f"updated {DOC_FILE}")
    return 0


def main(argv=None):
    parser = argparse.ArgumentParser(
        prog="vendor.py", description=__doc__.split("\n\n")[0],
        epilog="See the top of tools/vendor.py and third_party/README.md for details.")
    sub = parser.add_subparsers(dest="command", required=True)

    p = sub.add_parser("check", help="compare vendored files with upstream plus patches")
    p.add_argument("names", nargs="*", help="components to check (default: all, plus THIRD_PARTY.md)")
    p.set_defaults(func=cmd_check)

    p = sub.add_parser("update", help="rewrite a component from upstream plus patches")
    p.add_argument("name")
    p.add_argument("ref", nargs="?", help="full commit hash, tag or branch (default: the pinned commit)")
    p.set_defaults(func=cmd_update)

    p = sub.add_parser("diff", help="record local edits of a component as a new patch")
    p.add_argument("name")
    p.add_argument("-m", "--message", required=True,
                   help="description; the first line also names the patch file")
    p.set_defaults(func=cmd_diff)

    p = sub.add_parser("docs", help=f"regenerate the component sections of {DOC_FILE}")
    p.add_argument("--check", action="store_true", help="only compare, exit 1 on a difference")
    p.set_defaults(func=cmd_docs)

    args = parser.parse_args(argv)
    # Keep stdout and stderr in order when both go to a CI log.
    sys.stdout.reconfigure(line_buffering=True)
    if shutil.which("git") is None:
        print("vendor.py: git is required", file=sys.stderr)
        return 2
    try:
        return args.func(args)
    except VendorError as e:
        print(f"vendor.py: {e}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())

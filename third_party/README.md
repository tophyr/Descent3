# Vendored third-party code

Vendored code is third-party source code that is copied into this repository
and built with it. The copies are checked in, so a normal build never needs the
network.

Each vendored component has a manifest, `UPSTREAM.toml`, in its directory
(`third_party/<name>/`, or a top-level directory such as `libmve/`). The
manifest records the upstream git repository and commit the files came from,
which upstream files were copied and where they are stored, and the changes
made in Descent 3, as patch files in the component's `patches/` directory.
`tools/vendor.py` uses the manifests to check the copies, to update them and
to generate the component sections of `THIRD_PARTY.md`.

The tool needs Python 3.11 or later and git 2.32 or later.

## Commands

```
python3 tools/vendor.py check [<name>...]
python3 tools/vendor.py update <name> [<ref>]
python3 tools/vendor.py diff <name> -m "<description>"
python3 tools/vendor.py docs [--check]
```

`check` fetches each component's upstream commit into a new temporary
directory, applies the patches in order and compares the result with the files
in the working tree. It also reports files in the component directory that the
manifest does not list, patch files that are not listed, and listed files that
are missing. Without names it checks every component and also compares
`THIRD_PARTY.md` with the manifests. It exits with status 1 if anything differs.
CI runs it on every pull request.

`update` writes upstream plus the patches into the working tree. With a ref (a
full commit hash, a tag or a branch name), it uses that upstream commit and
records it in the manifest. If a patch no longer applies, it names the patch,
shows the error from `git apply` and writes nothing; fix or remove the patch and
run it again. Afterwards it runs `check` on the component.

`diff` records edits to the checked-in copy. It compares the working tree with
upstream plus the existing patches, writes the difference to a new patch file
`patches/NNNN-<description>.patch` and adds it to the end of the manifest's
`patches` list. It then rebuilds the component to confirm that the patches
reproduce the working tree. A typical change is: edit the vendored file, run
`diff`, run `docs`, commit.

`docs` rewrites the part of `THIRD_PARTY.md` between the `BEGIN` and `END`
comments from the manifests. Sections for code that is not vendored (plog) are
written by hand outside that part.

## How files are compared

Files are compared in the form git stores them. Before comparing, both the
expected content and the working tree file are converted with
`git hash-object --path=<local path>`, which applies the line ending rules of
`.gitattributes` (and `core.autocrlf`) for the local path. For example,
`*.c text eol=lf` makes git store `.c` files with LF line endings, so an
upstream `.c` file with CRLF line endings matches the LF copy here. Files
without a line ending rule (such as `.inc`, `.def` and `LICENSE`) are compared
byte for byte. Any other difference, including whitespace, is reported as a
modification.

Patches are made and applied against that stored form, so they contain the
line endings git stores. `.gitattributes` marks `patches/*.patch` as `-text`
so their bytes are not converted on checkout.

Only tracked files and untracked files that are not ignored by `.gitignore`
count as files in a component directory.

## Upstream content

Fetched upstream content is treated as data. Each run fetches the commit into a
new temporary bare repository with the user's and the system's git configuration
disabled, and reads the listed files with `git cat-file`. Nothing from upstream
is checked out, built or run. Only `https://` URLs are accepted, and symbolic
links and submodules upstream are not copied.

## Manifest fields

| Field | Meaning |
|---|---|
| `url` | Upstream git repository (`https://`). |
| `commit` | Full 40-character hash of the upstream commit. |
| `version` | Optional version string, shown in `THIRD_PARTY.md`. |
| `fork` | `true` for a component that has diverged so far from upstream that it is no longer compared with it. The manifest then only records its origin; `check` confirms that the commit and the upstream paths exist and that the file list is complete. A fork has no `patches`. |
| `local_files` | Files in the component directory that are not from upstream, such as `CMakeLists.txt`. |
| `patches` | Patch file names in `patches/`, applied in order with `git apply -p1`. Required unless `fork = true`; use `[]` for none. |
| `license_file` | The local file that holds the license text. |
| `[files]` | Upstream path = local path relative to the component directory. |
| `[version_check]` | Optional. `upstream_file` and a `regex` with one group; `check` requires the group to match `version` in that upstream file at `commit`. |
| `[[stamps]]` | Optional. `file` (relative to the repository root) must contain the line `line`, where `{version}` is replaced by `version` and `{commit}` by `commit`. Used to tie generated code to the upstream version and commit that generated it. |
| `[doc]` | `title`, `summary`, and optional `notes`, `license` and `license_text` for `THIRD_PARTY.md`. |

A patch file may start with a description. `git apply` ignores the text before
the first `diff --git` line.

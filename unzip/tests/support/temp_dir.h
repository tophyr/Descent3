/*
 * Descent 3
 * Copyright (C) 2026 Descent Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <filesystem>
#include <string>

#include "zip_builder.h"

// A temporary directory named after the running test, created on construction and removed on destruction.
class TempDir {
public:
  TempDir();
  ~TempDir();
  TempDir(const TempDir &) = delete;
  TempDir &operator=(const TempDir &) = delete;

  // Whether the directory was created.
  bool created() const { return created_; }

  // Path of `leaf` inside the directory.
  std::string PathFor(const std::string &leaf) const;

  // Writes `contents` to `leaf` inside the directory and returns its path.
  std::string WriteFile(const std::string &leaf, const Bytes &contents) const;

private:
  std::filesystem::path dir_;
  bool created_ = false;
};

// The whole contents of the file at `path`.
Bytes ReadWholeFile(const std::string &path);

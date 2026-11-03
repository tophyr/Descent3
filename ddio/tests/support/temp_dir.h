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

// A directory under the system temp directory for the running test, removed on destruction. The name includes the
// PID because ctest may run the tests of this executable in parallel processes.
class TempDir {
public:
  TempDir();
  ~TempDir();
  TempDir(const TempDir &) = delete;
  TempDir &operator=(const TempDir &) = delete;

  const std::filesystem::path &path() const { return m_path; }

  // Writes `contents` to the file `name` inside the directory and returns its path.
  std::filesystem::path Touch(const std::string &name, const std::string &contents = "") const;

private:
  std::filesystem::path m_path;
};

// Returns the whole contents of the file at `p`.
std::string ReadAll(const std::filesystem::path &p);

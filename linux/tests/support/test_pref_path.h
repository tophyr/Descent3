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

// Directory returned by the test stub of ddio_GetPrefPath(), which oeLnxAppDatabase uses to locate its registry file.
extern std::filesystem::path g_test_pref_path;

class ScopedTempDir {
public:
  ScopedTempDir();
  ~ScopedTempDir();
  ScopedTempDir(const ScopedTempDir &) = delete;
  ScopedTempDir &operator=(const ScopedTempDir &) = delete;

  const std::filesystem::path &path() const { return m_path; }

private:
  std::filesystem::path m_path;
};

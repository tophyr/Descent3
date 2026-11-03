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

#include "test_pref_path.h"

#include <random>
#include <string>
#include <system_error>

std::filesystem::path g_test_pref_path;

std::filesystem::path ddio_GetPrefPath(const char *, const char *) { return g_test_pref_path; }

ScopedTempDir::ScopedTempDir() {
  std::random_device rd;
  std::uniform_int_distribution<unsigned long long> dist;
  do {
    m_path = std::filesystem::temp_directory_path() / ("d3_linux_tests_" + std::to_string(dist(rd)));
  } while (!std::filesystem::create_directory(m_path));
}

ScopedTempDir::~ScopedTempDir() {
  std::error_code ec;
  std::filesystem::remove_all(m_path, ec);
}

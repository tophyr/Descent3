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

#include "temp_dir.h"

#include <fstream>
#include <iterator>
#include <system_error>

#include <gtest/gtest.h>

#include "ddio.h"

namespace fs = std::filesystem;

TempDir::TempDir() {
  const std::string test_name = testing::UnitTest::GetInstance()->current_test_info()->name();
  m_path = fs::temp_directory_path() / ("ddio_tests_" + std::to_string(ddio_GetPID()) + "_" + test_name);
  fs::remove_all(m_path);
  fs::create_directories(m_path);
}

TempDir::~TempDir() {
  std::error_code ec;
  fs::remove_all(m_path, ec);
}

fs::path TempDir::Touch(const std::string &name, const std::string &contents) const {
  fs::path p = m_path / name;
  std::ofstream(p, std::ios::binary) << contents;
  return p;
}

std::string ReadAll(const fs::path &p) {
  std::ifstream in(p, std::ios::binary);
  return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

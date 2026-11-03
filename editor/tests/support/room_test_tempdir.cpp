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

#include "room_test_tempdir.h"

#include <system_error>

#include <gtest/gtest.h>

namespace room_test {

TempDir::TempDir() {
  dir_ = std::filesystem::temp_directory_path() /
         ("editor_tests_" + std::to_string(::testing::UnitTest::GetInstance()->random_seed()) + "_" +
          ::testing::UnitTest::GetInstance()->current_test_info()->name());
  std::filesystem::create_directories(dir_);
}

TempDir::~TempDir() {
  std::error_code ec;
  std::filesystem::remove_all(dir_, ec);
}

std::string TempDir::Path(const char *name) const { return (dir_ / name).string(); }

} // namespace room_test

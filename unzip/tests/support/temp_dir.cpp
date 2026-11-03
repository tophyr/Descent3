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

#include <cctype>
#include <fstream>
#include <iterator>
#include <system_error>

#include <gtest/gtest.h>

TempDir::TempDir() {
  const auto *info = ::testing::UnitTest::GetInstance()->current_test_info();
  std::string leaf = std::string("d3_unzip_tests_") + info->test_suite_name() + "_" + info->name();
  for (char &c : leaf)
    if (!isalnum(static_cast<unsigned char>(c)))
      c = '_';
  dir_ = std::filesystem::temp_directory_path() / leaf;
  std::filesystem::remove_all(dir_);
  created_ = std::filesystem::create_directories(dir_);
}

TempDir::~TempDir() {
  std::error_code ec;
  std::filesystem::remove_all(dir_, ec);
}

std::string TempDir::PathFor(const std::string &leaf) const { return (dir_ / leaf).string(); }

std::string TempDir::WriteFile(const std::string &leaf, const Bytes &contents) const {
  std::string path = PathFor(leaf);
  std::ofstream f(path, std::ios::binary);
  f.write(reinterpret_cast<const char *>(contents.data()), static_cast<std::streamsize>(contents.size()));
  EXPECT_TRUE(f.good());
  return path;
}

Bytes ReadWholeFile(const std::string &path) {
  std::ifstream f(path, std::ios::binary);
  return Bytes(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
}

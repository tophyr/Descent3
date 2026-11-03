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
#include <fstream>
#include <random>
#include <string>
#include <string_view>

class TestTempDir {
public:
  TestTempDir() {
    std::random_device rd;
    std::mt19937_64 rng(rd());
    auto base = std::filesystem::temp_directory_path();
    do {
      path_ = base / ("d3_test_" + std::to_string(rng()));
    } while (!std::filesystem::create_directory(path_));
  }
  ~TestTempDir() {
    std::error_code ec;
    std::filesystem::remove_all(path_, ec);
  }
  TestTempDir(const TestTempDir &) = delete;
  TestTempDir &operator=(const TestTempDir &) = delete;

  const std::filesystem::path &path() const { return path_; }

  std::filesystem::path Write(const std::string &name, std::string_view contents) const {
    auto file = path_ / name;
    std::ofstream out(file, std::ios::binary);
    out.write(contents.data(), static_cast<std::streamsize>(contents.size()));
    return file;
  }

private:
  std::filesystem::path path_;
};

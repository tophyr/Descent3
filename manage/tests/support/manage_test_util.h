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

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <random>
#include <string>
#include <vector>

#include <gtest/gtest.h>

class ManageTempDirTest : public ::testing::Test {
protected:
  void SetUp() override {
    std::random_device rd;
    const std::filesystem::path base(::testing::TempDir());
    do {
      dir_ = base / ("manage_" + std::to_string(rd()));
    } while (!std::filesystem::create_directories(dir_));
  }

  void TearDown() override {
    std::error_code ec;
    std::filesystem::remove_all(dir_, ec);
  }

  std::filesystem::path Path(const std::string &name) const { return dir_ / name; }

  static std::vector<uint8_t> ReadAllBytes(const std::filesystem::path &p) {
    std::ifstream in(p, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
  }

  std::filesystem::path dir_;
};

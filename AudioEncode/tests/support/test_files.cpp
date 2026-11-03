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

#include "test_files.h"

#include <cstdio>
#include <random>
#include <string>

#include <gtest/gtest.h>

namespace fs = std::filesystem;

ScratchDir::~ScratchDir() {
  if (dir_.empty())
    return;
  std::error_code ec;
  fs::remove_all(dir_, ec);
}

bool ScratchDir::Create() {
  std::random_device rd;
  dir_ = fs::temp_directory_path() / ("d3_audioencode_tests_" + std::to_string(rd()) + std::to_string(rd()));
  return fs::create_directories(dir_);
}

std::vector<uint8_t> ReadFile(const fs::path &p) {
  std::vector<uint8_t> out;
  FILE *f = fopen(p.string().c_str(), "rb");
  if (!f)
    return out;
  int c;
  while ((c = getc(f)) != EOF)
    out.push_back(static_cast<uint8_t>(c));
  fclose(f);
  return out;
}

void WriteRawPcm(const fs::path &p, const std::vector<int16_t> &pcm) {
  FILE *f = fopen(p.string().c_str(), "wb");
  ASSERT_NE(f, nullptr);
  for (int16_t s : pcm) {
    const uint16_t u = static_cast<uint16_t>(s);
    putc(u & 0xFF, f);
    putc(u >> 8, f);
  }
  fclose(f);
}

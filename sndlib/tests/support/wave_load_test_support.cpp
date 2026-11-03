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

#include "wave_load_test_support.h"

#include <fstream>
#include <random>
#include <system_error>

#include "ssl_lib.h"

int16_t To16(uint8_t u) { return static_cast<int16_t>((u - 128) * 256); }

TempWaveDir::TempWaveDir() {
  std::random_device rd;
  dir_ = std::filesystem::temp_directory_path() / ("d3_sndlib_tests_" + std::to_string(rd()) + std::to_string(rd()));
  std::filesystem::create_directories(dir_);
}

TempWaveDir::~TempWaveDir() {
  std::error_code ec;
  std::filesystem::remove_all(dir_, ec);
}

std::string TempWaveDir::Write(const std::string &name, const std::vector<uint8_t> &bytes) const {
  std::filesystem::path p = dir_ / name;
  std::ofstream f(p, std::ios::binary);
  f.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
  return p.string();
}

bool Load(const std::string &path, int slot, bool high_quality, float volume, bool load_data) {
  return SoundLoadWaveFile(path.c_str(), volume, slot, high_quality, load_data) != 0;
}

std::vector<int> Samples16(int slot) {
  const sound_file_info &sf = SoundFiles[slot];
  return std::vector<int>(sf.sample_16bit, sf.sample_16bit + sf.sample_length);
}

std::vector<int> Samples8(int slot) {
  const sound_file_info &sf = SoundFiles[slot];
  return std::vector<int>(sf.sample_8bit, sf.sample_8bit + sf.sample_length);
}

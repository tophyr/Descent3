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
#include <random>
#include <string>
#include <system_error>
#include <vector>

class ScopedTempDir {
public:
  ScopedTempDir() {
    std::random_device rd;
    std::filesystem::path base = std::filesystem::temp_directory_path();
    for (int attempt = 0; attempt < 100; attempt++) {
      std::filesystem::path candidate = base / ("d3_stream_audio_tests_" + std::to_string(rd()));
      if (std::filesystem::create_directory(candidate)) {
        m_path = candidate;
        return;
      }
    }
  }
  ~ScopedTempDir() {
    if (!m_path.empty()) {
      std::error_code ec;
      std::filesystem::remove_all(m_path, ec);
    }
  }
  ScopedTempDir(const ScopedTempDir &) = delete;
  ScopedTempDir &operator=(const ScopedTempDir &) = delete;

  const std::filesystem::path &path() const { return m_path; }

  // The path of the file `name` inside the directory.
  std::filesystem::path File(const char *name) const { return m_path / name; }

private:
  std::filesystem::path m_path;
};

inline void WriteFileBytes(const std::filesystem::path &path, const std::vector<uint8_t> &bytes) {
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  out.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

inline std::vector<uint8_t> ReadFileBytes(const std::filesystem::path &path) {
  std::ifstream in(path, std::ios::binary);
  return std::vector<uint8_t>((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

inline std::vector<uint8_t> Repeat(const std::vector<uint8_t> &v, int times) {
  std::vector<uint8_t> out;
  for (int i = 0; i < times; i++)
    out.insert(out.end(), v.begin(), v.end());
  return out;
}

inline std::vector<uint8_t> MakePattern(size_t size, uint8_t seed) {
  std::vector<uint8_t> data(size);
  uint32_t x = seed * 2654435761u + 1;
  for (auto &b : data) {
    x = x * 1664525u + 1013904223u;
    b = static_cast<uint8_t>(x >> 24);
  }
  return data;
}

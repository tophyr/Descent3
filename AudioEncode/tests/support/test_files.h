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

#ifndef AUDIOENCODE_TESTS_TEST_FILES_H
#define AUDIOENCODE_TESTS_TEST_FILES_H

#include <cstdint>
#include <filesystem>
#include <vector>

// A uniquely named directory under the system temp directory, removed with its contents on destruction.
class ScratchDir {
public:
  ScratchDir() = default;
  ~ScratchDir();
  ScratchDir(const ScratchDir &) = delete;
  ScratchDir &operator=(const ScratchDir &) = delete;

  // Creates the directory; returns whether it was created.
  bool Create();

  std::filesystem::path Path(const char *name) const { return dir_ / name; }

private:
  std::filesystem::path dir_;
};

// The whole contents of a file, or nothing if it cannot be opened.
std::vector<uint8_t> ReadFile(const std::filesystem::path &p);

// Writes samples as raw 16-bit little-endian PCM.
void WriteRawPcm(const std::filesystem::path &p, const std::vector<int16_t> &pcm);

#endif

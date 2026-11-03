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

#ifndef BITMAP_TESTS_TEST_SUPPORT_H
#define BITMAP_TESTS_TEST_SUPPORT_H

#include <cstdint>
#include <filesystem>
#include <set>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "bitmap.h"

class ByteWriter {
public:
  ByteWriter &u8(uint8_t v) {
    bytes.push_back(v);
    return *this;
  }
  ByteWriter &u16le(uint16_t v) { return u8(v & 0xFF).u8(v >> 8); }
  ByteWriter &u16be(uint16_t v) { return u8(v >> 8).u8(v & 0xFF); }
  ByteWriter &u32le(uint32_t v) { return u16le(v & 0xFFFF).u16le(v >> 16); }
  ByteWriter &u32be(uint32_t v) { return u16be(v >> 16).u16be(v & 0xFFFF); }
  ByteWriter &str(const char *s) {
    while (*s)
      u8(static_cast<uint8_t>(*s++));
    return *this;
  }
  ByteWriter &fill(size_t n, uint8_t v) {
    bytes.insert(bytes.end(), n, v);
    return *this;
  }
  ByteWriter &append(const std::vector<uint8_t> &other) {
    bytes.insert(bytes.end(), other.begin(), other.end());
    return *this;
  }
  size_t size() const { return bytes.size(); }

  std::vector<uint8_t> bytes;
};

constexpr uint16_t Rgb1555(int r, int g, int b) {
  return static_cast<uint16_t>(0x8000 | (r << 10) | (g << 5) | b);
}
constexpr uint16_t Argb4444(int a, int r, int g, int b) {
  return static_cast<uint16_t>((a << 12) | (r << 8) | (g << 4) | b);
}

// True when a 1555 pixel has its opaque bit clear.
bool IsTransparent1555(uint16_t pixel);

// An opaque 1555 colour that differs between neighbouring source pixels of a chunked bitmap.
uint16_t ChunkPattern(int x, int y);

// Allocates a w x h bitmap of `format` holding `pixels`. The caller owns the handle.
int AllocBitmap(int w, int h, const std::vector<uint16_t> &pixels, int format = BITMAP_FORMAT_1555,
                bool room_for_mips = false);

// Saves `handle` as dir/roundtrip.ogf and loads it back; returns the loaded handle (owned by the caller) or -1.
int SaveAndReload(const std::filesystem::path &dir, int handle, int mipped);

// Saves a 2x2 bitmap of `fill`, named `filename`, as dir/filename and returns its path.
std::filesystem::path WriteOgf(const std::filesystem::path &dir, const std::string &filename,
                               uint16_t fill = Rgb1555(1, 2, 3));

// Like WriteOgf, but the name stored inside the OGF is `stored_name` instead of the file name.
std::filesystem::path WriteOgfStoringName(const std::filesystem::path &dir, const std::string &filename,
                                          const std::string &stored_name, uint16_t fill = Rgb1555(1, 2, 3));

// Writes `bytes` to `path`, replacing any existing file.
void WriteBytes(const std::filesystem::path &path, const std::vector<uint8_t> &bytes);

// Returns the whole contents of the file at `path`.
std::vector<uint8_t> ReadAll(const std::filesystem::path &path);

// Reads the big-endian 32-bit value at `offset`.
uint32_t ReadBe32(const std::vector<uint8_t> &bytes, size_t offset);

class BitmapLibTest : public ::testing::Test {
protected:
  void SetUp() override;
  void TearDown() override;

  // Remembers a handle for cleanup and returns it unchanged.
  int Track(int handle);

  int MakeBitmap(int w, int h, const std::vector<uint16_t> &pixels, int format = BITMAP_FORMAT_1555,
                 bool room_for_mips = false);

  static std::vector<uint16_t> Pixels(int handle, int mip = 0);

  std::filesystem::path temp_dir;

private:
  std::set<int> tracked_;
};

#endif

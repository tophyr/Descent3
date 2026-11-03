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

#include "test_font.h"

#include <cstdio>

#include "grdefs.h"

namespace twodlib_test {

namespace {

// Font file flags (font.cpp)
constexpr int16_t FT_COLOR = 1;
constexpr int16_t FT_PROPORTIONAL = 2;

void Put16(std::vector<uint8_t> &out, int16_t v) {
  out.push_back((uint8_t)(v & 0xFF));
  out.push_back((uint8_t)((v >> 8) & 0xFF));
}

void Put32(std::vector<uint8_t> &out, uint32_t v) {
  for (int i = 0; i < 4; i++)
    out.push_back((uint8_t)((v >> (8 * i)) & 0xFF));
}

} // namespace

int GlyphWidth(int ch) {
  switch (ch) {
  case ' ':
    return 3;
  case 'A':
    return 4;
  case 'B':
    return 5;
  case 'C':
    return 6;
  default:
    return 2;
  }
}

uint16_t GlyphPixel(int ch, int col, int row) { return (uint16_t)(OPAQUE_FLAG16 | (ch << 8) | (col << 4) | row); }

std::vector<uint8_t> BuildFontFile() {
  std::vector<uint8_t> out;
  Put32(out, 0xfeedbaba);
  Put16(out, 6); // widest char
  Put16(out, kGlyphHeight);
  Put16(out, FT_COLOR | FT_PROPORTIONAL);
  Put16(out, 0); // baseline
  out.push_back(kMinAscii);
  out.push_back(kMaxAscii);
  char name[32] = {};
  std::snprintf(name, sizeof(name), "%s", kFontName);
  out.insert(out.end(), name, name + 32);

  for (int ch = kMinAscii; ch <= kMaxAscii; ch++)
    Put16(out, (int16_t)GlyphWidth(ch));

  std::vector<uint8_t> raw;
  for (int ch = kMinAscii; ch <= kMaxAscii; ch++)
    for (int row = 0; row < kGlyphHeight; row++)
      for (int col = 0; col < GlyphWidth(ch); col++)
        Put16(raw, (int16_t)GlyphPixel(ch, col, row));
  Put32(out, (uint32_t)raw.size());
  out.insert(out.end(), raw.begin(), raw.end());
  return out;
}

} // namespace twodlib_test

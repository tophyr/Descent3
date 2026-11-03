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
#include <vector>

class WavBuilder {
public:
  WavBuilder &Raw(const std::vector<uint8_t> &bytes) {
    body_.insert(body_.end(), bytes.begin(), bytes.end());
    return *this;
  }
  WavBuilder &Fmt(uint16_t format_tag, uint16_t channels, uint32_t rate, uint16_t bits) {
    std::vector<uint8_t> c;
    Put16(c, format_tag);
    Put16(c, channels);
    Put32(c, rate);
    Put32(c, rate * channels * (bits / 8)); // average bytes per second
    Put16(c, static_cast<uint16_t>(channels * (bits / 8))); // block align
    Put16(c, bits);
    return Chunk("fmt ", c);
  }
  WavBuilder &PcmFmt(uint16_t bits) { return Fmt(1, 1, 22050, bits); }
  WavBuilder &Data8(const std::vector<uint8_t> &samples) { return Chunk("data", samples); }
  WavBuilder &Data16(const std::vector<int16_t> &samples) {
    std::vector<uint8_t> c;
    for (int16_t s : samples)
      Put16(c, static_cast<uint16_t>(s));
    return Chunk("data", c);
  }
  WavBuilder &Chunk(const char id[5], const std::vector<uint8_t> &payload) {
    body_.insert(body_.end(), id, id + 4);
    Put32(body_, static_cast<uint32_t>(payload.size()));
    body_.insert(body_.end(), payload.begin(), payload.end());
    return *this;
  }

  std::vector<uint8_t> Build(const char riff[5] = "RIFF", const char wave[5] = "WAVE") const {
    std::vector<uint8_t> out(riff, riff + 4);
    Put32(out, static_cast<uint32_t>(body_.size() + 4));
    out.insert(out.end(), wave, wave + 4);
    out.insert(out.end(), body_.begin(), body_.end());
    return out;
  }

private:
  static void Put16(std::vector<uint8_t> &v, uint16_t x) {
    v.push_back(static_cast<uint8_t>(x & 0xFF));
    v.push_back(static_cast<uint8_t>(x >> 8));
  }
  static void Put32(std::vector<uint8_t> &v, uint32_t x) {
    for (int i = 0; i < 4; ++i)
      v.push_back(static_cast<uint8_t>((x >> (8 * i)) & 0xFF));
  }
  std::vector<uint8_t> body_;
};

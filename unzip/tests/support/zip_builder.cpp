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

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>
#include <zlib.h>

#include "zip_builder.h"

// Incompressible: forces deflate to emit stored blocks and makes the compressed size larger than the reader's
// internal I/O chunk sizes.
Bytes RandomBytes(size_t n, uint32_t seed) {
  Bytes out(n);
  uint32_t x = seed ? seed : 1;
  for (auto &b : out) {
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    b = static_cast<uint8_t>(x);
  }
  return out;
}

Bytes TextBytes(size_t n) {
  static const char kText[] = "The quick brown fox jumps over the lazy dog. Descent 3 mission data. ";
  Bytes out(n);
  for (size_t i = 0; i < n; ++i)
    out[i] = static_cast<uint8_t>(kText[i % (sizeof(kText) - 1)]);
  return out;
}

Bytes SampleBytes(size_t n, bool random) {
  return random ? RandomBytes(n, static_cast<uint32_t>(n) + 1) : TextBytes(n);
}

Bytes ToBytes(const std::string &s) { return Bytes(s.begin(), s.end()); }

Bytes AsBytes(const std::vector<char> &v) { return Bytes(v.begin(), v.end()); }

Bytes RawDeflate(const Bytes &in) {
  z_stream s{};
  EXPECT_EQ(deflateInit2(&s, Z_BEST_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY), Z_OK);
  Bytes out(deflateBound(&s, static_cast<uLong>(in.size())) + 16);
  s.next_in = const_cast<Bytef *>(in.data());
  s.avail_in = static_cast<uInt>(in.size());
  s.next_out = out.data();
  s.avail_out = static_cast<uInt>(out.size());
  EXPECT_EQ(deflate(&s, Z_FINISH), Z_STREAM_END);
  out.resize(s.total_out);
  deflateEnd(&s);
  return out;
}

uint32_t Crc(const Bytes &data) {
  return static_cast<uint32_t>(crc32(0L, data.data(), static_cast<uInt>(data.size())));
}

void Put16(Bytes &b, uint16_t v) {
  b.push_back(static_cast<uint8_t>(v));
  b.push_back(static_cast<uint8_t>(v >> 8));
}
void Put32(Bytes &b, uint32_t v) {
  for (int i = 0; i < 4; ++i)
    b.push_back(static_cast<uint8_t>(v >> (8 * i)));
}
void PutBytes(Bytes &b, const Bytes &v) { b.insert(b.end(), v.begin(), v.end()); }
void PutStr(Bytes &b, const std::string &v) { b.insert(b.end(), v.begin(), v.end()); }

Archive BuildZip(std::vector<Entry> entries, const std::string &archive_comment) {
  Archive ar;
  Bytes &b = ar.bytes;
  std::vector<uint32_t> offsets;

  for (auto &e : entries) {
    if (!e.payload_set)
      e.payload = (e.method == kMethodDeflate) ? RawDeflate(e.data) : e.data;
    const uint32_t crc = Crc(e.data);
    const uint16_t flags = e.data_descriptor ? 0x0008 : 0;

    offsets.push_back(static_cast<uint32_t>(b.size()));
    Put32(b, 0x04034b50);
    Put16(b, e.version_needed);
    Put16(b, flags);
    Put16(b, e.method);
    Put16(b, 0); // time
    Put16(b, 0); // date
    Put32(b, e.data_descriptor ? 0 : crc);
    Put32(b, e.data_descriptor ? 0 : static_cast<uint32_t>(e.payload.size()));
    Put32(b, e.data_descriptor ? 0 : static_cast<uint32_t>(e.data.size()));
    Put16(b, static_cast<uint16_t>(e.name.size()));
    Put16(b, static_cast<uint16_t>(e.local_extra.size()));
    PutStr(b, e.name);
    PutBytes(b, e.local_extra);
    PutBytes(b, e.payload);
    if (e.data_descriptor) {
      Put32(b, 0x08074b50);
      Put32(b, crc);
      Put32(b, static_cast<uint32_t>(e.payload.size()));
      Put32(b, static_cast<uint32_t>(e.data.size()));
    }
  }

  ar.cd_offset = static_cast<uint32_t>(b.size());
  for (size_t i = 0; i < entries.size(); ++i) {
    const Entry &e = entries[i];
    Put32(b, 0x02014b50);
    Put16(b, 20); // version made by (MS-DOS host)
    Put16(b, e.version_needed);
    Put16(b, e.data_descriptor ? 0x0008 : 0);
    Put16(b, e.method);
    Put16(b, 0); // time
    Put16(b, 0); // date
    Put32(b, Crc(e.data));
    Put32(b, e.override_sizes ? e.central_compressed_size : static_cast<uint32_t>(e.payload.size()));
    Put32(b, e.override_sizes ? e.central_uncompressed_size : static_cast<uint32_t>(e.data.size()));
    Put16(b, static_cast<uint16_t>(e.name.size()));
    Put16(b, static_cast<uint16_t>(e.central_extra.size()));
    Put16(b, static_cast<uint16_t>(e.comment.size()));
    Put16(b, 0); // disk number start
    Put16(b, 0); // internal attributes
    Put32(b, 0); // external attributes
    Put32(b, e.override_offset ? e.central_local_offset : offsets[i]);
    PutStr(b, e.name);
    PutBytes(b, e.central_extra);
    PutStr(b, e.comment);
  }
  const uint32_t cd_size = static_cast<uint32_t>(b.size()) - ar.cd_offset;

  Put32(b, 0x06054b50);
  Put16(b, 0); // this disk
  Put16(b, 0); // disk with central directory
  Put16(b, static_cast<uint16_t>(entries.size()));
  Put16(b, static_cast<uint16_t>(entries.size()));
  Put32(b, cd_size);
  Put32(b, ar.cd_offset);
  Put16(b, static_cast<uint16_t>(archive_comment.size()));
  PutStr(b, archive_comment);
  return ar;
}

Entry Stored(const std::string &name, Bytes data) {
  Entry e;
  e.name = name;
  e.data = std::move(data);
  e.method = kMethodStored;
  return e;
}

Entry Deflated(const std::string &name, Bytes data) {
  Entry e;
  e.name = name;
  e.data = std::move(data);
  e.method = kMethodDeflate;
  return e;
}

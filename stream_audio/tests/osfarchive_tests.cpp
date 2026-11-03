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
#include <cstring>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "streamaudio.h"
#include "support/osf_writer.h"
#include "support/test_helpers.h"

namespace {

class OSFArchiveTest : public ::testing::Test {
protected:
  void SetUp() override { ASSERT_FALSE(m_dir.path().empty()); }

  ScopedTempDir m_dir;
};

class OSFArchiveRateTest : public OSFArchiveTest, public ::testing::WithParamInterface<uint32_t> {};

TEST_P(OSFArchiveRateTest, WriteThenReadPreservesHeaderAndData) {
  const uint32_t rate = GetParam();
  const std::vector<uint8_t> data = MakePattern(5000, 1);
  const auto path = m_dir.File("roundtrip.osf");
  WriteOsf(path, data, OSF_DIGIACM_STRM, SAF_16BIT_S, rate, 12345, "Theme Song");

  OSFArchive osf;
  ASSERT_TRUE(osf.Open(path));
  EXPECT_TRUE(osf.Opened());
  EXPECT_EQ(osf.StreamType(), OSF_DIGITAL_STRM);
  EXPECT_EQ(osf.StreamComp(), OSF_DIGIACM_STRM);
  EXPECT_EQ(osf.StreamFormat(), SAF_16BIT_S);
  EXPECT_EQ(osf.StreamRate(), rate);
  EXPECT_EQ(osf.StreamLength(), data.size());
  EXPECT_EQ(static_cast<const tOSFDigiHdr *>(osf.StreamHeader())->measure, 12345u);
  EXPECT_STREQ(osf.StreamName(), "Theme Song");

  std::vector<uint8_t> back(data.size());
  EXPECT_EQ(osf.Read(back.data(), static_cast<int>(back.size())), static_cast<int>(back.size()));
  EXPECT_EQ(back, data);
}

INSTANTIATE_TEST_SUITE_P(StandardRates, OSFArchiveRateTest, ::testing::Values(11025u, 22050u, 44100u));

// Built byte-by-byte in the shipped .osf layout rather than with the writer, so existing game data stays readable:
// data, then a 128-byte trailer of
//   "OSF1", type, compression, format, rate code (kHz: 11/22/44), measure (int32 LE), length (int32 LE),
// with a 32-byte title at offset 64 within the trailer.
TEST_F(OSFArchiveTest, ReadsFileInShippedLayout) {
  const std::vector<uint8_t> data = MakePattern(300, 2);
  const uint32_t measure = 0x00012345;
  const uint32_t length = static_cast<uint32_t>(data.size());

  std::vector<uint8_t> trailer(128, 0);
  std::memcpy(&trailer[0], "OSF1", 4);
  trailer[4] = 0;    // digital stream
  trailer[5] = 1;    // ACM compression
  trailer[6] = 0x01; // 16-bit mono
  trailer[7] = 22;   // 22 kHz
  for (int i = 0; i < 4; i++) {
    trailer[8 + i] = static_cast<uint8_t>(measure >> (8 * i));
    trailer[12 + i] = static_cast<uint8_t>(length >> (8 * i));
  }
  std::memcpy(&trailer[64], "Hand Built", 10);

  std::vector<uint8_t> file = data;
  file.insert(file.end(), trailer.begin(), trailer.end());
  const auto path = m_dir.File("handbuilt.osf");
  WriteFileBytes(path, file);

  OSFArchive osf;
  ASSERT_TRUE(osf.Open(path));
  EXPECT_EQ(osf.StreamType(), OSF_DIGITAL_STRM);
  EXPECT_EQ(osf.StreamComp(), OSF_DIGIACM_STRM);
  EXPECT_EQ(osf.StreamFormat(), SAF_16BIT_M);
  EXPECT_EQ(osf.StreamRate(), 22050u);
  EXPECT_EQ(osf.StreamLength(), length);
  EXPECT_EQ(static_cast<const tOSFDigiHdr *>(osf.StreamHeader())->measure, measure);
  EXPECT_STREQ(osf.StreamName(), "Hand Built");

  std::vector<uint8_t> back(data.size());
  ASSERT_EQ(osf.Read(back.data(), static_cast<int>(back.size())), static_cast<int>(back.size()));
  EXPECT_EQ(back, data);
}

TEST_F(OSFArchiveTest, RewindRestartsReadingFromStartOfData) {
  const std::vector<uint8_t> data = MakePattern(1000, 3);
  const auto path = m_dir.File("rewind.osf");
  WriteOsf(path, data, OSF_DIGIACM_STRM, SAF_16BIT_M, 22050, 1, "r");

  OSFArchive osf;
  ASSERT_TRUE(osf.Open(path));
  std::vector<uint8_t> first(700);
  ASSERT_EQ(osf.Read(first.data(), 700), 700);

  osf.Rewind();
  std::vector<uint8_t> again(data.size());
  ASSERT_EQ(osf.Read(again.data(), static_cast<int>(again.size())), static_cast<int>(again.size()));
  EXPECT_EQ(again, data);
}

TEST_F(OSFArchiveTest, RejectsFileWithoutOsfTrailerAndStaysReusable) {
  const auto bogus = m_dir.File("bogus.osf");
  WriteFileBytes(bogus, MakePattern(400, 4));
  const auto good = m_dir.File("good.osf");
  WriteOsf(good, MakePattern(64, 5), OSF_DIGIACM_STRM, SAF_16BIT_M, 22050, 1, "good");

  OSFArchive osf;
  EXPECT_FALSE(osf.Open(bogus));
  EXPECT_FALSE(osf.Opened());

  ASSERT_TRUE(osf.Open(good));
  EXPECT_STREQ(osf.StreamName(), "good");
}

TEST_F(OSFArchiveTest, RejectsNonDigitalStreamType) {
  const std::vector<uint8_t> data = MakePattern(100, 6);
  std::vector<uint8_t> trailer(128, 0);
  std::memcpy(&trailer[0], "OSF1", 4);
  trailer[4] = 1; // not OSF_DIGITAL_STRM, the only stream type there is
  trailer[7] = 22;
  std::vector<uint8_t> file = data;
  file.insert(file.end(), trailer.begin(), trailer.end());
  const auto path = m_dir.File("type1.osf");
  WriteFileBytes(path, file);

  OSFArchive osf;
  EXPECT_FALSE(osf.Open(path));
  EXPECT_FALSE(osf.Opened());
}

} // namespace

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

#include "grtext_test_support.h"

#include <algorithm>
#include <cstdio>
#include <memory>
#include <random>

#include "bitmap.h"
#include "cfile.h"
#include "grtext.h"
#include "renderer.h"

namespace {

void PutShort(std::vector<uint8_t> &out, int16_t v) {
  out.push_back(static_cast<uint8_t>(v & 0xff));
  out.push_back(static_cast<uint8_t>((v >> 8) & 0xff));
}

void PutInt(std::vector<uint8_t> &out, uint32_t v) {
  for (int i = 0; i < 4; i++)
    out.push_back(static_cast<uint8_t>((v >> (8 * i)) & 0xff));
}

constexpr int16_t kFlagColor = 1, kFlagProportional = 2, kFlagKerned = 4, kFlagFFI2 = 32;

} // namespace

void WriteFontFile(const std::filesystem::path &path, const TestFont &font) {
  std::vector<uint8_t> out;
  PutInt(out, 0xfeedbaba);
  PutShort(out, font.width);
  PutShort(out, font.height);
  int16_t flags = (font.color ? kFlagColor : 0) | (font.widths.empty() ? 0 : kFlagProportional) |
                  (font.kerning.empty() ? 0 : kFlagKerned) | (font.ffi2 ? kFlagFFI2 : 0);
  PutShort(out, flags);
  PutShort(out, 0); // baseline
  out.push_back(font.min_ascii);
  out.push_back(font.max_ascii);
  char name[32] = "test font";
  out.insert(out.end(), name, name + sizeof(name));

  if (font.ffi2) {
    PutShort(out, font.tracking);
    out.insert(out.end(), 62, 0);
  }
  for (uint8_t w : font.widths)
    PutShort(out, w);
  if (!font.kerning.empty()) {
    PutShort(out, static_cast<int16_t>(font.kerning.size()));
    for (const auto &k : font.kerning) {
      out.push_back(k.first);
      out.push_back(k.second);
      out.push_back(static_cast<uint8_t>(k.delta));
    }
  }

  std::vector<uint8_t> pixels;
  for (int i = 0; i < font.NumGlyphs(); i++) {
    int w = font.GlyphWidth(font.min_ascii + i);
    size_t size = font.color ? size_t(w) * font.height * 2 : size_t((w + 7) / 8) * font.height;
    std::vector<uint8_t> glyph(size, 0);
    if (i < static_cast<int>(font.glyph_data.size()) && !font.glyph_data[i].empty()) {
      EXPECT_EQ(font.glyph_data[i].size(), size) << "bad test glyph data for glyph " << i;
      std::copy_n(font.glyph_data[i].begin(), std::min(size, font.glyph_data[i].size()), glyph.begin());
    }
    pixels.insert(pixels.end(), glyph.begin(), glyph.end());
  }
  PutInt(out, static_cast<uint32_t>(pixels.size()));
  out.insert(out.end(), pixels.begin(), pixels.end());

  std::unique_ptr<FILE, decltype(&fclose)> fp(fopen(path.u8string().c_str(), "wb"), &fclose);
  ASSERT_TRUE(fp) << "cannot create " << path;
  ASSERT_EQ(fwrite(out.data(), 1, out.size(), fp.get()), out.size());
}

bms_bitmap GameBitmaps[MAX_BITMAPS];
static std::vector<uint16_t> Bitmap_pixels[MAX_BITMAPS];

int bm_AllocBitmap(int w, int h, int /*add_mem*/) {
  for (int i = 1; i < MAX_BITMAPS; i++) { // 0 is BAD_BITMAP_HANDLE
    if (!GameBitmaps[i].used) {
      Bitmap_pixels[i].assign(size_t(w) * h, 0xdead);
      GameBitmaps[i] = {};
      GameBitmaps[i].used = 1;
      GameBitmaps[i].width = static_cast<uint16_t>(w);
      GameBitmaps[i].height = static_cast<uint16_t>(h);
      GameBitmaps[i].data16 = Bitmap_pixels[i].data();
      return i;
    }
  }
  return BAD_BITMAP_HANDLE;
}

void bm_FreeBitmap(int handle) {
  GameBitmaps[handle] = {};
  Bitmap_pixels[handle].clear();
}

uint16_t *bm_data(int handle, int /*miplevel*/) { return GameBitmaps[handle].data16; }
int bm_rowsize(int handle, int /*miplevel*/) { return GameBitmaps[handle].width * 2; }
int bm_h(int handle, int /*miplevel*/) { return GameBitmaps[handle].height; }

uint16_t BitmapPixel(int bm_handle, int x, int y) {
  const bms_bitmap &bm = GameBitmaps[bm_handle];
  EXPECT_TRUE(bm.used);
  EXPECT_TRUE(x >= 0 && x < bm.width && y >= 0 && y < bm.height);
  return bm.data16[y * bm.width + x];
}

std::vector<DrawnGlyph> &DrawnGlyphs() {
  static std::vector<DrawnGlyph> glyphs;
  return glyphs;
}

void rend_DrawFontCharacter(int bm_handle, int x1, int y1, int x2, int y2, float u, float v, float w, float h) {
  DrawnGlyphs().push_back({bm_handle, x1, y1, x2, y2, u, v, w, h});
}

void rend_SetTextureType(texture_type) {}
void rend_SetOverlayType(uint8_t) {}
void rend_SetFiltering(int8_t) {}
void rend_SetLighting(light_state) {}
void rend_SetAlphaType(int8_t) {}
void rend_SetColorModel(color_model) {}
void rend_SetZBufferState(int8_t) {}
void rend_SetAlphaValue(uint8_t) {}
void rend_SetFlatColor(ddgr_color) {}
void rend_DrawScaledBitmap(int, int, int, int, int, float, float, float, float, int, const float *) {}

// grfont_KeyToAscii's only dependency on the keyboard code; the real one would drag in ddio's SDL window glue.

int ddio_KeyToAscii(int code) { return code; }

void EnsureGrtextInitialized() {
  static bool initialized = false;
  if (!initialized) {
    grtext_Init();
    initialized = true;
  }
}

void GrtextTest::SetUp() {
  std::random_device rd;
  dir_ = std::filesystem::temp_directory_path() /
         ("grtext_tests_" + std::to_string(rd()) + "_" + ::testing::UnitTest::GetInstance()->current_test_info()->name());
  std::filesystem::create_directories(dir_);
  // grfont_Load keeps the name it is given in a 32 character buffer, so fonts are loaded by bare file name.
  cf_SetSearchPath(dir_);

  EnsureGrtextInitialized();
  grfont_Reset();
  grtext_Reset();
  grtext_SetParameters(kLeft, kTop, kRight, kBottom, kTabSpaces);
  DrawnGlyphs().clear();
}

void GrtextTest::TearDown() {
  grtext_Reset();
  grfont_Reset();
  DrawnGlyphs().clear();
  cf_ClearAllSearchPaths();
  std::error_code ec;
  std::filesystem::remove_all(dir_, ec);
}

std::filesystem::path GrtextTest::WriteFont(const TestFont &font) {
  std::filesystem::path path = dir_ / ("font" + std::to_string(font_counter_++) + ".fnt");
  WriteFontFile(path, font);
  return path;
}

int GrtextTest::LoadFont(const TestFont &font) {
  std::filesystem::path path = WriteFont(font);
  int handle = grfont_Load(path.filename().u8string().c_str());
  // grfont_Load's failure value is indistinguishable from handle 0, so check that the font really arrived.
  EXPECT_EQ(grfont_GetHeight(handle), font.height) << "font " << path << " did not load";
  return handle;
}

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

// The tests only exercise memory surfaces, so these do nothing except provide real
// pixel storage for the font bitmaps that grFont allocates.

#include <cstdint>
#include <vector>

#include "bitmap.h"
#include "Ddgr.h"
#include "renderer.h"

namespace {
struct StubBitmap {
  int w, h;
  std::vector<uint16_t> pixels;
};

// Intentionally leaked so it outlives grFont::close_system(), which runs from atexit.
std::vector<StubBitmap> &Bitmaps() {
  static auto *bitmaps = new std::vector<StubBitmap>;
  return *bitmaps;
}

StubBitmap *Lookup(int handle) {
  if (handle < 0 || handle >= (int)Bitmaps().size())
    return nullptr;
  return &Bitmaps()[handle];
}
} // namespace

int bm_AllocBitmap(int w, int h, int add_mem) {
  Bitmaps().push_back(StubBitmap{w, h, std::vector<uint16_t>((size_t)w * h + add_mem)});
  return (int)Bitmaps().size() - 1;
}

void bm_FreeBitmap(int handle) {}

uint16_t *bm_data(int handle, int miplevel) {
  StubBitmap *bm = Lookup(handle);
  return bm ? bm->pixels.data() : nullptr;
}

int bm_w(int handle, int miplevel) {
  StubBitmap *bm = Lookup(handle);
  return bm ? bm->w : 0;
}

int bm_h(int handle, int miplevel) {
  StubBitmap *bm = Lookup(handle);
  return bm ? bm->h : 0;
}

int bm_rowsize(int handle, int miplevel) {
  StubBitmap *bm = Lookup(handle);
  return bm ? bm->w * 2 : 0;
}

int bm_bpp(int handle) { return BPP_16; }

int bm_format(int handle) { return BITMAP_FORMAT_1555; }

float ddgr_GetAspectRatio() { return 1.0f; }
bool ddgr_surf_Create(ddgr_surface *sf) { return false; }
void ddgr_surf_Destroy(ddgr_surface *sf) {}
bool ddgr_surf_Lock(ddgr_surface *sf, void **ptr, int *rowsize) { return false; }
bool ddgr_surf_Unlock(ddgr_surface *sf, void *ptr) { return false; }
void ddgr_surf_AttachHandle(ddgr_surface *sf, unsigned handle) {}
bool ddgr_surf_InitVideo(ddgr_surface *sf) { return false; }
void ddgr_surf_CloseVideo(ddgr_surface *sf) {}
bool ddgr_surf_FlipVideo(ddgr_surface *sf) { return false; }
void ddgr_surf_Clear(ddgr_surface *dsf, ddgr_color col, int l, int t, int w, int h) {}
bool ddgr_surf_Blt(ddgr_surface *dsf, int dx, int dy, ddgr_surface *ssf, int sx, int sy, int sw, int sh) {
  return false;
}

void rend_SetSoftwareParameters(float aspect, int width, int height, int pitch, uint8_t *framebuffer) {}
void rend_SetTextureType(texture_type) {}
void rend_SetOverlayType(uint8_t type) {}
void rend_SetFiltering(int8_t state) {}
void rend_SetLighting(light_state) {}
void rend_SetAlphaType(int8_t) {}
void rend_SetColorModel(color_model) {}
void rend_SetZBufferState(int8_t state) {}
void rend_SetAlphaValue(uint8_t val) {}
void rend_SetFlatColor(ddgr_color color) {}
void rend_SetCharacterParameters(ddgr_color color1, ddgr_color color2, ddgr_color color3, ddgr_color color4) {}
void rend_FillRect(ddgr_color color, int x1, int y1, int x2, int y2) {}
void rend_SetPixel(ddgr_color color, int x, int y) {}
ddgr_color rend_GetPixel(int x, int y) { return 0; }
void rend_DrawFontCharacter(int bm_handle, int x1, int y1, int x2, int y2, float u, float v, float w, float h) {}
void rend_DrawLine(int x1, int y1, int x2, int y2) {}
void rend_FillCircle(ddgr_color col, int x, int y, int rad) {}
void rend_DrawCircle(int x, int y, int rad) {}
void rend_Flip() {}

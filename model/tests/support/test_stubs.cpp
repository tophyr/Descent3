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

#include <algorithm>
#include <cctype>
#include <cstring>

#include "3d.h"
#include "gamesequence.h"
#include "gametexture.h"
#include "objinfo.h"
#include "polymodel.h"
#include "renderer.h"

#include "test_stubs.h"

namespace model_test {
std::map<std::string, int> texture_bitmap_names;
std::map<std::string, int> texture_names;
int data_error_count = 0;

void ResetStubs() {
  texture_bitmap_names.clear();
  texture_names.clear();
  data_error_count = 0;
  for (auto &t : GameTextures) {
    std::memset(&t, 0, sizeof(t));
    t.alpha = 1.0f;
  }
}
} // namespace model_test

// Descent3/GameLoop.cpp
float Gametime = 0.0f;

// Descent3/gamesequence.cpp
int paged_in_count = 0;
int paged_in_num = 0;

// Descent3/objinfo.cpp
object_info Object_info[MAX_OBJECT_IDS];

// Descent3/gametexture.cpp
texture GameTextures[MAX_TEXTURES];

// Like the real lookups, these match names case-insensitively (keys are stored lowercase).
static int LookupLowercase(const std::map<std::string, int> &names, const char *name) {
  std::string key(name);
  std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return std::tolower(c); });
  auto it = names.find(key);
  return it == names.end() ? -1 : it->second;
}

int FindTextureBitmapName(const char *name) { return LookupLowercase(model_test::texture_bitmap_names, name); }

int FindTextureName(const char *name) { return LookupLowercase(model_test::texture_names, name); }

// manage/manage.cpp
void DataError(const char *, ...) { model_test::data_error_count++; }

// model/newstyle.cpp
int RenderPolygonModel(poly_model *, uint32_t) { return 0; }
void RenderSubmodel(poly_model *, bsp_info *, uint32_t) {}
float ComputeDefaultSize(int, int, float *size_ptr) {
  if (size_ptr)
    *size_ptr = 0.0f;
  return 0.0f;
}

// renderer
void g3_StartInstanceMatrix(vector *, matrix *) {}
void g3_DoneInstance() {}
void rend_SetLighting(light_state) {}
void rend_SetColorModel(color_model) {}
void rend_SetOverlayType(uint8_t) {}
void rend_SetAlphaType(int8_t) {}

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

// Link-time stand-ins for the parts of the UI library that need a renderer or an application
// (UIDraw.cpp, UISystem.cpp, grtext).

#include <cstring>

#include <gtest/gtest.h>

#include "UIlib.h"
#include "ddio.h"
#include "grtext.h"
#include "mem.h"

// On Windows the mem library refuses to allocate until mem_Init() has created its heap, and
// UITextItem allocates its string through it.
namespace {
class MemEnvironment : public ::testing::Environment {
public:
  void SetUp() override { mem_Init(); }
};
[[maybe_unused]] ::testing::Environment *const kMemEnvironment =
    ::testing::AddGlobalTestEnvironment(new MemEnvironment);
} // namespace

tUIInput UI_input;
tUIOutput UI_output;
int UI_screen_width = 640, UI_screen_height = 480;
float UI_aspect_x = 1.0f;
float UI_aspect_y = 1.0f;
float UIFrameTime = 0.0f;

void ui_AddWindow(UIWindow *) {}
void ui_RemoveWindow(UIWindow *) {}

void ui_StartDraw(int, int, int, int) {}
void ui_EndDraw() {}
void ui_SetTextClip(int, int, int, int) {}
void ui_ResetTextClip() {}
void ui_DrawSetAlpha(uint8_t) {}
void ui_DrawLine(ddgr_color, int, int, int, int) {}
void ui_DrawBox(ddgr_color, int, int, int, int) {}
void ui_DrawLTBox(ddgr_color, ddgr_color, int, int, int, int) {}
void ui_DrawRect(ddgr_color, int, int, int, int) {}
void ui_DrawBitmap(UIBitmapItem *, int, int, uint8_t) {}
int ui_GetBitmapWidth(UIBitmapItem *) { return 0; }
int ui_GetBitmapHeight(UIBitmapItem *) { return 0; }
void ui_DrawSetFont(int) {}
int ui_DrawGetFont() { return 0; }
int ui_GetFontHeight() { return 12; }
void ui_SetCharAlpha(uint8_t) {}
int ui_GetTextWidth(const char *text) { return 8 * (int)strlen(text); }
int ui_GetTextHeight(const char *) { return 12; }
void ui_DrawSetTextType(int) {}
void ui_DrawString(ddgr_color, int, int, const char *) {}
void ui_DrawCenteredString(ddgr_color, int, int, const char *) {}

// grtext: the real implementation only adds font-specific uppercasing on top of ddio's table lookup.
int grfont_KeyToAscii(int, int key) { return ddio_KeyToAscii(key); }

// Referenced by ddio's SDL keyboard backend, which the linker pulls in alongside the key tables.
struct SDL_Window;
SDL_Window *GSDLWindow = nullptr;
bool Game_fullscreen = false;
void rend_SetFullScreen(bool) {}

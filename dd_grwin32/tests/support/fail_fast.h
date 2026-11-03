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

#include <SDL3/SDL_assert.h>

// ddgr reports failures through Error(), which in debug builds pops a modal message box (hanging
// an unattended test run) and then calls exit(0) (which ctest would count as a pass).
extern bool g_test_running;

[[noreturn]] void FailFast(const char *why);

void OnDebugBreak();

void OnExitDuringTest();

SDL_AssertState SDLCALL OnAssert(const SDL_AssertData *data, void *);

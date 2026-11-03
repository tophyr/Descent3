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

#include "fail_fast.h"

#include <cstdio>
#include <cstdlib>

bool g_test_running = false;

[[noreturn]] void FailFast(const char *why) {
  std::fprintf(stderr, "dd_grwin32 test aborted: %s\n", why);
  std::fflush(stderr);
  std::_Exit(1);
}

void OnDebugBreak() { FailFast("Error()/Int3() raised by the library"); }

void OnExitDuringTest() {
  if (g_test_running)
    FailFast("exit() called while a test was running");
}

SDL_AssertState SDLCALL OnAssert(const SDL_AssertData *data, void *) {
  std::fprintf(stderr, "ASSERT(%s) failed at %s:%d\n", data->condition, data->filename, data->linenum);
  FailFast("library assertion failed");
}

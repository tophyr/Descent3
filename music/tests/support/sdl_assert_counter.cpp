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

#include "sdl_assert_counter.h"

#include <SDL3/SDL_assert.h>
#include <gtest/gtest.h>

// Int3()/ASSERT() go through SDL_assert, whose default handler pops up a modal
// dialog in debug builds. Count them instead so a test can never hang.
std::atomic<int> g_assert_count{0};

namespace {

SDL_AssertState SDLCALL CountingAssertHandler(const SDL_AssertData *, void *) {
  ++g_assert_count;
  return SDL_ASSERTION_IGNORE;
}

class AssertCatcher : public ::testing::Environment {
public:
  void SetUp() override { SDL_SetAssertionHandler(CountingAssertHandler, nullptr); }
  void TearDown() override { SDL_SetAssertionHandler(nullptr, nullptr); }
};

const ::testing::Environment *const g_assert_env = ::testing::AddGlobalTestEnvironment(new AssertCatcher);

} // namespace

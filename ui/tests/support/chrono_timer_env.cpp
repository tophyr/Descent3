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

#include <gtest/gtest.h>

#include "chrono_timer.h"

namespace {
// Starts the clock once per test process, as the game does at startup.
class ChronoTimerEnvironment : public ::testing::Environment {
public:
  void SetUp() override { D3::ChronoTimer::Initialize(); }
};
[[maybe_unused]] ::testing::Environment *const kChronoTimerEnvironment =
    ::testing::AddGlobalTestEnvironment(new ChronoTimerEnvironment);
} // namespace

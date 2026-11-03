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

#include <cstdio>
#include <cstdlib>

// Error() ends in exit(0), so a regression that exits where it should return
// would otherwise end the test process with status 0 mid-test, which ctest
// reports as a pass.
class MustReturnGuard {
public:
  MustReturnGuard() {
    static const bool registered = (std::atexit(Tripwire), true);
    (void)registered;
    armed_ = true;
  }
  ~MustReturnGuard() { armed_ = false; }
  MustReturnGuard(const MustReturnGuard &) = delete;
  MustReturnGuard &operator=(const MustReturnGuard &) = delete;

private:
  static void Tripwire() {
    if (!armed_)
      return;
    std::fputs("FAILURE: program exited from a call that was expected to return\n", stderr);
    std::fflush(stderr);
    std::_Exit(3);
  }
  static inline bool armed_ = false;
};

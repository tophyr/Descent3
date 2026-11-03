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

#include "ddio.h"

// Stand-ins for the platform key handler (lnxkey.cpp), which key.cpp calls into.
bool ddio_InternalKeyInit(ddio_init_info *) { return false; }
void ddio_InternalKeyClose() {}
float ddio_InternalKeyDownTime(uint8_t) { return 0.0f; }
void ddio_InternalResetKey(uint8_t) {}

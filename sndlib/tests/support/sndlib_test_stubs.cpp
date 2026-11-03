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

// Link-time stand-ins for game globals that sndlib's loader code references but that
// live in the Descent3 executable.

#include "door.h"
#include "gamesequence.h"
#include "gametexture.h"
#include "objinfo.h"
#include "room.h"
#include "ship.h"
#include "soundpage.h"
#include "weapon.h"

// gamesequence.cpp: level paging progress counters bumped by SoundLoadWaveFile.
int paged_in_count = 0;
int paged_in_num = 0;

// Tables scanned by RemapAllSoundObjects (soundload.cpp).
weapon Weapons[MAX_WEAPONS];
object_info Object_info[MAX_OBJECT_IDS];
ship Ships[MAX_SHIPS];
room Rooms[MAX_ROOMS];
int Highest_room_index = -1;
door Doors[MAX_DOORS];
texture GameTextures[MAX_TEXTURES];

// soundpage.cpp: only used by LoadSound(), which the tests do not exercise.
int mng_GetGuaranteedSoundPage(char *, CFILE *) { return -1; }

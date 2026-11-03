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

// Link-time stand-ins for the game-side symbols the manage sources reference.
// The manage library is normally linked into the Descent3 executable, which
// provides them; the tests never touch them.

#include <cstdio>
#include <cstdlib>
#include <filesystem>

#include "bitmap.h"
#include "door.h"
#include "gamefile.h"
#include "gametexture.h"
#include "megacell.h"
#include "objinfo.h"
#include "polymodel.h"
#include "ship.h"
#include "soundload.h"
#include "ssl_lib.h"
#include "vclip.h"
#include "weapon.h"

class oeAppDatabase;

namespace {
[[noreturn]] void Unexpected(const char *fn) {
  std::fprintf(stderr, "manage_tests: unexpected call to stub %s\n", fn);
  std::abort();
}
} // namespace

#define STUB(fn) Unexpected(fn)

bool Dedicated_server = false;
bool Running_editor = false;
oeAppDatabase *Database = nullptr;

bms_bitmap GameBitmaps[MAX_BITMAPS];
door Doors[MAX_DOORS];
gamefile Gamefiles[MAX_GAMEFILES];
megacell Megacells[MAX_MEGACELLS];
object_info Object_info[MAX_OBJECT_IDS];
poly_model Poly_models[MAX_POLY_MODELS];
ship Ships[MAX_SHIPS];
sound_file_info SoundFiles[MAX_SOUND_FILES];
sound_info Sounds[MAX_SOUNDS];
texture GameTextures[MAX_TEXTURES];
vclip GameVClips[MAX_VCLIPS];
weapon Weapons[MAX_WEAPONS];

const char *GetStringFromTable(int) { STUB("GetStringFromTable"); }
void InitMessage(const char *, float) { STUB("InitMessage"); }
void RemapEverything() { STUB("RemapEverything"); }

int AllocDoor() { STUB("AllocDoor"); }
void FreeDoor(int) { STUB("FreeDoor"); }
int FindDoorName(const char *) { STUB("FindDoorName"); }

int AllocGamefile() { STUB("AllocGamefile"); }
int FindGamefileName(char *) { STUB("FindGamefileName"); }

int AllocMegacell() { STUB("AllocMegacell"); }
int FindMegacellName(char *) { STUB("FindMegacellName"); }

int AllocObjectID(int, bool, bool, bool) { STUB("AllocObjectID"); }
void FreeObjectID(int) { STUB("FreeObjectID"); }
int FindObjectIDName(const char *) { STUB("FindObjectIDName"); }

int AllocShip() { STUB("AllocShip"); }
void FreeShip(int) { STUB("FreeShip"); }
int FindShipName(const char *) { STUB("FindShipName"); }

int AllocSound() { STUB("AllocSound"); }
void FreeSound(int) { STUB("FreeSound"); }
int FindSoundName(const char *) { STUB("FindSoundName"); }
int LoadSoundFile(const char *, float, bool) { STUB("LoadSoundFile"); }
void FreeSoundFile(int) { STUB("FreeSoundFile"); }

int AllocTexture() { STUB("AllocTexture"); }
void FreeTexture(int) { STUB("FreeTexture"); }
int FindTextureName(const char *) { STUB("FindTextureName"); }
int LoadTextureImage(const char *, int *, int, int, int, int) { STUB("LoadTextureImage"); }
int AllocateProceduralForTexture(int) { STUB("AllocateProceduralForTexture"); }
void AllocateStaticProceduralsForTexture(int, int) { STUB("AllocateStaticProceduralsForTexture"); }
void FreeProceduralForTexture(int) { STUB("FreeProceduralForTexture"); }

int AllocWeapon() { STUB("AllocWeapon"); }
void FreeWeapon(int) { STUB("FreeWeapon"); }
int FindWeaponName(const char *) { STUB("FindWeaponName"); }

int AllocLoadVClip(const char *, int, int, int, int) { STUB("AllocLoadVClip"); }
void FreeVClip(int) { STUB("FreeVClip"); }

int bm_AllocLoadFileBitmap(const char *, int, int) { STUB("bm_AllocLoadFileBitmap"); }
void bm_FreeBitmap(int) { STUB("bm_FreeBitmap"); }

int LoadPolyModel(const std::filesystem::path &, int) { STUB("LoadPolyModel"); }
void FreePolyModel(int) { STUB("FreePolyModel"); }
void FreePolymodelData(int) { STUB("FreePolymodelData"); }
poly_model *GetPolymodelPointer(int) { STUB("GetPolymodelPointer"); }

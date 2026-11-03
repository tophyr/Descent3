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

// Link-time stand-ins for the game-side symbols that findintersection.cpp and physics.cpp
// reference. The tested functions never touch them, but other functions in the same
// translation units do. The real definitions live in the Descent3 executable (and in the
// bitmap/model libraries and collide.cpp), which would drag in the whole game.

#include <cstdint>

#include "attach.h"
#include "bitmap.h"
#include "collide.h"
#include "D3ForceFeedback.h"
#include "demofile.h"
#include "doorway.h"
#include "findintersection.h"
#include "fireball.h"
#include "game.h"
#include "gametexture.h"
#include "multi.h"
#include "object.h"
#include "objinfo.h"
#include "physics.h"
#include "player.h"
#include "polymodel.h"
#include "room.h"
#include "rtperformance.h"
#include "terrain.h"
#include "viseffect.h"
#include "weapon.h"

object Objects[MAX_OBJECTS];
object *Player_object = nullptr;
int Num_big_objects = 0;
int16_t BigObjectList[MAX_BIG_OBJECTS];
object_info Object_info[MAX_OBJECT_IDS];
room Rooms[MAX_ROOMS + MAX_PALETTE_ROOMS];
int Highest_room_index = -1;
player Players[MAX_PLAYERS];
int Player_num = 0;
poly_model Poly_models[MAX_POLY_MODELS];
texture GameTextures[MAX_TEXTURES];
bms_bitmap GameBitmaps[MAX_BITMAPS];
terrain_segment Terrain_seg[(TERRAIN_WIDTH + 1) * (TERRAIN_DEPTH + 1)];
terrain_tex_segment Terrain_tex_seg[TERRAIN_TEX_WIDTH * TERRAIN_TEX_DEPTH];
terrain_normals *TerrainNormals[MAX_TERRAIN_LOD];
uint8_t *Terrain_max_height_int[7];
vis_effect *VisEffects = nullptr;
netgame_info Netgame;

float Frametime = 0.0f;
float Gametime = 0.0f;
int Game_mode = 0;
uint32_t Demo_flags = 0;
int Game_do_flying_sim = 1;
int Game_do_walking_sim = 1;
int Game_do_vis_sim = 1;
bool Level_powerups_ignore_wind = false;
int force_field_bounce_texture[MAX_FORCE_FIELD_BOUNCE_TEXTURES];
float force_field_bounce_multiplier[MAX_FORCE_FIELD_BOUNCE_TEXTURES];
bool Tracking_FVI = false;

// collide.cpp and newstyle_fi.cpp are not compiled into this test.
uint8_t CollisionResult[MAX_OBJECT_TYPES][MAX_OBJECT_TYPES];
uint8_t CollisionRayResult[MAX_OBJECT_TYPES];
void ConvertEulerToAxisAmount(vector *e, vector *n, float *w) {}
bool collide_object_with_wall(object *A, float hitspeed, int hitseg, int hitwall, vector *hitpt, vector *wall_normal,
                              float hit_dot) {
  return true;
}
void collide_two_objects(object *A, object *B, vector *collision_point, vector *collision_normal,
                         fvi_info *hit_info) {}
void bump_obj_against_fixed(object *obj, vector *collision_point, vector *collision_normal) {}
bool PolyCollideObject(object *obj) { return false; }

// player (in Players[]) embeds an Inventory (Descent3/Inventory.cpp).
Inventory::Inventory() {}
Inventory::~Inventory() {}

// rtperformance needs cfile and the game, so it is not linked. These are only referenced
// when USE_RTP is defined.
#ifdef USE_RTP
tRTFrameInfo RTP_SingleFrame;
uint8_t Runtime_performance_enabled = 0;
INT64 rtp_GetClock() { return 0; }
#endif

int ObjCreate(uint8_t type, uint16_t id, int roomnum, vector *pos, const matrix *orient, int parent_handle) {
  return -1;
}
object *ObjGet(int handle) { return nullptr; }
void ObjSetPos(object *obj, vector *pos, int roomnum, matrix *orient, bool f_update_attached_children) {}
void ObjSetOrient(object *obj, const matrix *orient) {}
void ObjSetAABB(object *obj) {}
void SetObjectDeadFlag(object *obj, bool tell_clients_to_remove, bool play_sound_on_clients) {}
void DestroyObject(object *objp, float explosion_mag, int death_flags) {}
void AttachUpdateSubObjects(object *obj) {}
bool ObjectsAreRelated(int o1, int o2) { return false; }
void SetModelAnglesAndPos(poly_model *po, float *normalized_time, uint32_t subobj_flags) {}
void SetNormalizedTimeObj(object *obj, float *normalized_time) {}
void VisEffectDelete(int visnum) {}
void VisEffectRelink(int visnum, int newroomnum) {}
void ForceEffectsPlay(int id, float *scale, vector *direction) {}
float DoorwayPosition(room *rp) { return 0.0f; }
int CheckTransparentPoint(const vector *pnt, const room *rp, const int facenum) { return 0; }
int GetTerrainRoomFromPos(vector *pos) { return -1; }
int GetTerrainCellFromPos(vector *pos) { return -1; }
int GetTextureBitmap(int handle, int framenum, bool force) { return 0; }

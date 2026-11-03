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
#include <cstdlib>

#include "test_support.h"

namespace mt_test {
float now = 0.0f;
int poll_ui_result = 0;
std::vector<SentPacket> sent;

void Reset() {
  now = 1000.0f;
  poll_ui_result = 0;
  sent.clear();
}
} // namespace mt_test

typedef void *(*mem_malloc_fp)(int size);
typedef void (*mem_free_fp)(void *memblock);
static void *StubMalloc(int size) { return std::malloc(size); }
static void StubFree(void *p) { std::free(p); }
mem_malloc_fp DLLmem_malloc = StubMalloc;
mem_free_fp DLLmem_free = StubFree;

// The real table lives in the DLL's string file; the tests only need formats
// that take the same arguments.
const char *GetString(int d) {
  switch (d) {
  case 69: // TXT_PXO_ERRORNOTONLINE
    return "NOTONLINE(%s)";
  case 70: // TXT_PXO_BADNICK
    return "BADNICK";
  case 71: // TXT_PXO_PRIVMSGTO
    return "TO(%s): %s";
  case 72: // TXT_PXO_PRIVMSGFROM
    return "FROM(%s): %s";
  default:
    return "";
  }
}

typedef float (*timer_GetTime_fp)();
static float StubGetTime() { return mt_test::now; }
timer_GetTime_fp DLLtimer_GetTime = StubGetTime;

typedef int (*PollUI_fp)();
static int StubPollUI() { return mt_test::poll_ui_result; }
PollUI_fp DLLPollUI = StubPollUI;

static bool dedicated_server = true;
bool *DLLDedicated_server = &dedicated_server;

typedef int (*nw_Asyncgethostbyname_fp)(uint32_t *ip, int command, const char *hostname);
static int StubAsyncgethostbyname(uint32_t *, int, const char *) { return -1; }
nw_Asyncgethostbyname_fp DLLnw_Asyncgethostbyname = StubAsyncgethostbyname;

typedef int (*nw_SendWithID_fp)(uint8_t id, uint8_t *data, int len, network_address *who_to);
static int StubSendWithID(uint8_t id, uint8_t *data, int len, network_address *who_to) {
  mt_test::SentPacket p;
  p.id = id;
  p.bytes.assign(data, data + len);
  p.to = *who_to;
  mt_test::sent.push_back(p);
  return len;
}
nw_SendWithID_fp DLLnw_SendWithID = StubSendWithID;

typedef void (*PacketHandler_fp)(uint8_t *data, int len, network_address *from);
typedef int (*nw_RegisterCallback_fp)(PacketHandler_fp, uint8_t id);
static int StubRegisterCallback(PacketHandler_fp, uint8_t) { return 1; }
nw_RegisterCallback_fp DLLnw_RegisterCallback = StubRegisterCallback;

typedef int (*nw_DoReceiveCallbacks_fp)();
static int StubDoReceiveCallbacks() { return 0; }
nw_DoReceiveCallbacks_fp DLLnw_DoReceiveCallbacks = StubDoReceiveCallbacks;

// Globals normally defined in con_dll.h / mtclient.cpp
int MT_Initialized = 0;
uint16_t DLLPXOPort = 0;
static char hosted_lobby_name[] = "#test_lobby";
char *DLLPXO_hosted_lobby_name = hosted_lobby_name;

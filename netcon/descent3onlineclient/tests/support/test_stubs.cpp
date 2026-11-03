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

// Link-time stand-ins for the symbols the online client normally receives from the game
// executable (via con_dll.h / DLLMultiInit) or defines in odtclient.cpp.

#include <cstdint>
#include <cstdlib>

#include "test_stubs.h"

typedef void *(*mem_malloc_fp)(int size);
typedef void (*mem_free_fp)(void *memblock);
typedef int (*nw_Asyncgethostbyname_fp)(uint32_t *ip, int command, const char *hostname);
typedef int (*PollUI_fp)();
typedef void (*Debug_ConsolePrintf_fp)(int n, const char *format, ...);

static void *TestMalloc(int size) { return std::malloc(size); }
static void TestFree(void *p) { std::free(p); }

mem_malloc_fp DLLmem_malloc = TestMalloc;
mem_free_fp DLLmem_free = TestFree;
// Only reached by ConnectToChatServer's name lookup, which the tests never get to.
nw_Asyncgethostbyname_fp DLLnw_Asyncgethostbyname = nullptr;
PollUI_fp DLLPollUI = nullptr;
Debug_ConsolePrintf_fp DLLDebug_ConsolePrintf = nullptr;

// The real string table lives in the .d3c hog.
const char *GetString(int d) {
  switch (d) {
  case 47:
    return TEST_FMT_NOT_ONLINE;
  case 49:
    return TEST_FMT_PRIVMSG_TO;
  case 50:
    return TEST_FMT_PRIVMSG_FROM;
  default:
    return "";
  }
}

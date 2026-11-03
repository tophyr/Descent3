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

// The memory hook test_stubs.cpp points at free().
extern void (*DLLmem_free)(void *);

// Formats the GetString() stub returns in place of the localised strings
// TXT_ODT_ERRORNOTONLINE, TXT_ODT_PRIVMSGTO and TXT_ODT_PRIVMSGFROM.
#define TEST_FMT_NOT_ONLINE "not-online %s"
#define TEST_FMT_PRIVMSG_TO "to %s: %s"
#define TEST_FMT_PRIVMSG_FROM "from %s: %s"

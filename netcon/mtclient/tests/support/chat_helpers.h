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

#include <optional>
#include <ostream>
#include <string>
#include <vector>

namespace mt_test {

std::optional<std::string> Parse(const std::string &line, int mode);

std::optional<std::string> Remote(const std::string &line);
std::optional<std::string> Local(const std::string &line);

// The nick our own channel messages are echoed with, or "" if the echo fails.
std::string CurrentNick();

std::vector<std::string> SplitWords(const char *s);

std::vector<std::string> Users();

struct Cmd {
  int command;
  std::string data;
};

std::vector<Cmd> DrainCommands();

std::vector<std::string> CommandData(const std::vector<Cmd> &cmds, int command);

bool HasCommand(const std::vector<Cmd> &cmds, int command);

struct ChannelEntry {
  std::string name;
  int users;
  std::string topic;
  bool operator==(const ChannelEntry &o) const { return name == o.name && users == o.users && topic == o.topic; }
};

void PrintTo(const ChannelEntry &e, std::ostream *os);

// The list is consumed by mtclient.cpp as '$'-separated records of
// "<channel> <users> <topic>".
std::vector<ChannelEntry> ParseChannelList(const char *list);

const char *ChannelList();

} // namespace mt_test

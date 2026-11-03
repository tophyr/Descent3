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

#include <algorithm>
#include <cstring>
#include <sstream>

#include "chat_helpers.h"
#include "chat_internals.h"
#include "chat_api.h"

namespace mt_test {

std::optional<std::string> Parse(const std::string &line, int mode) {
  std::vector<char> buf(line.begin(), line.end());
  buf.push_back('\0');
  const char *r = ParseIRCMessage(buf.data(), mode);
  if (!r)
    return std::nullopt;
  return std::string(r);
}

std::optional<std::string> Remote(const std::string &line) { return Parse(line, MSG_REMOTE); }
std::optional<std::string> Local(const std::string &line) { return Parse(line, MSG_LOCAL); }

std::string CurrentNick() {
  auto r = Local("PRIVMSG #c :x");
  if (!r)
    return "";
  auto open = r->find('[');
  auto close = r->find(']');
  return r->substr(open + 1, close - open - 1);
}

std::vector<std::string> SplitWords(const char *s) {
  std::vector<std::string> out;
  std::istringstream in(s);
  std::string w;
  while (in >> w)
    out.push_back(w);
  return out;
}

std::vector<std::string> Users() {
  Socket_connected = 1;
  const char *list = GetChatUserList();
  Socket_connected = 0;
  std::vector<std::string> users = list ? SplitWords(list) : std::vector<std::string>{};
  std::sort(users.begin(), users.end());
  return users;
}

std::vector<Cmd> DrainCommands() {
  std::vector<Cmd> out;
  while (Chat_command *c = GetChatCommandFromQueue()) {
    Cmd cmd{c->command, ""};
    if (c->command != CC_KICKED && c->command != CC_DISCONNECTED)
      cmd.data.assign(c->data, strnlen(c->data, sizeof(c->data)));
    out.push_back(cmd);
  }
  return out;
}

std::vector<std::string> CommandData(const std::vector<Cmd> &cmds, int command) {
  std::vector<std::string> out;
  for (const auto &c : cmds)
    if (c.command == command)
      out.push_back(c.data);
  return out;
}

bool HasCommand(const std::vector<Cmd> &cmds, int command) {
  return std::any_of(cmds.begin(), cmds.end(), [&](const Cmd &c) { return c.command == command; });
}

void PrintTo(const ChannelEntry &e, std::ostream *os) { *os << "{" << e.name << ", " << e.users << ", " << e.topic << "}"; }

std::vector<ChannelEntry> ParseChannelList(const char *list) {
  std::vector<ChannelEntry> out;
  std::string s(list);
  size_t pos = s.find('$');
  while (pos != std::string::npos) {
    size_t next = s.find('$', pos + 1);
    std::string rec = s.substr(pos + 1, next == std::string::npos ? std::string::npos : next - pos - 1);
    std::istringstream in(rec);
    ChannelEntry e;
    in >> e.name >> e.users;
    std::getline(in, e.topic);
    auto b = e.topic.find_first_not_of(' ');
    auto t = e.topic.find_last_not_of(' ');
    e.topic = b == std::string::npos ? "" : e.topic.substr(b, t - b + 1);
    out.push_back(e);
    pos = next;
  }
  return out;
}

const char *ChannelList() {
  Socket_connected = 1;
  const char *p = GetChannelList();
  Socket_connected = 0;
  return p;
}

} // namespace mt_test

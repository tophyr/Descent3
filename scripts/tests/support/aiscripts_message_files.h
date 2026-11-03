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

#include <filesystem>
#include <string>

namespace aiscripts_test {

// A temporary directory, unique to the running test, for message files. Removed on destruction.
class MessageFileDir {
public:
  MessageFileDir();
  ~MessageFileDir();
  MessageFileDir(const MessageFileDir &) = delete;
  MessageFileDir &operator=(const MessageFileDir &) = delete;

  const std::filesystem::path &path() const { return m_dir; }

  // Writes `contents` to `file_name` in the directory and returns its path.
  std::string Write(const std::string &file_name, const std::string &contents) const;

  // Writes `contents` to a message file and loads it with LoadMessageFile.
  bool Load(const std::string &contents) const;

private:
  std::filesystem::path m_dir;
};

// What LookupMessage returns for a name that isn't in the loaded message file.
std::string NotFoundMessage();

} // namespace aiscripts_test

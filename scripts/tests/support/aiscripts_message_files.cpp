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

#include "aiscripts_message_files.h"

#include <fstream>
#include <random>
#include <system_error>

#include <gtest/gtest.h>

#include "aiscripts_test_support.h"

namespace aiscripts_test {

MessageFileDir::MessageFileDir() {
  const auto *info = ::testing::UnitTest::GetInstance()->current_test_info();
  m_dir = std::filesystem::temp_directory_path() /
          (std::string("aiscripts_msg_") + info->name() + "_" + std::to_string(std::random_device{}()));
  std::filesystem::create_directories(m_dir);
}

MessageFileDir::~MessageFileDir() {
  std::error_code ec;
  std::filesystem::remove_all(m_dir, ec);
}

std::string MessageFileDir::Write(const std::string &file_name, const std::string &contents) const {
  const std::filesystem::path path = m_dir / file_name;
  std::ofstream out(path, std::ios::binary);
  out << contents;
  out.close();
  return path.string();
}

bool MessageFileDir::Load(const std::string &contents) const {
  return LoadMessageFile(Write("test.msg", contents).c_str());
}

std::string NotFoundMessage() { return LookupMessage("No Message Has This Name"); }

} // namespace aiscripts_test

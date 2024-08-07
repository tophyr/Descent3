/*
* Descent 3
* Copyright (C) 2024 Descent Developers
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
#include <filesystem>
#include <vector>
#include <gtest/gtest.h>
#include "cfile.h"

TEST(D3, CFileIO) {
  int lib_handle = cf_OpenLibrary("TestDir/test.hog");
  CFILE *file_handle = cfopen("lowercase.txt", "rb");
  char buf[5];
  cf_ReadString(buf, 5, file_handle);
  EXPECT_STREQ(buf, "TEST");
  cf_Rewind(file_handle);

  EXPECT_EQ(cf_ReadByte(file_handle), 84);
  cf_Rewind(file_handle);

  EXPECT_EQ(cf_ReadShort(file_handle), 17748);
  cf_Rewind(file_handle);

  EXPECT_EQ(cf_ReadInt(file_handle), 1414743380);
  cf_Rewind(file_handle);

  cf_CloseLibrary(lib_handle);
}

TEST(D3, CFileLibrary) {
  // First pass - without search path in "TestDir" (i.e. not search actual files in directory)
  // Second pass - with search path (files in directory goes first)
  for (int i = 0; i < 2; i++) {
    if (i != 0) {
      EXPECT_EQ(cf_SetSearchPath("TestDir"), true);
    }

    int lib_handle = cf_OpenLibrary("TestDir/test.hog");
    EXPECT_NE(lib_handle, 0);

    CFILE *file_handle = cf_OpenFileInLibrary("lowercase.txt", lib_handle);
    EXPECT_NE(file_handle, nullptr);
    file_handle = cfopen("lowercase.txt", "rb");
    EXPECT_NE(file_handle, nullptr);
    // Length in hog is 4, in directory is 0
    EXPECT_EQ(cfilelength(file_handle), i == 0 ? 4 : 0);
    EXPECT_EQ(cf_CalculateFileCRC(file_handle), i == 0 ? 4008350648 : 0);

    cf_ClearAllSearchPaths();
    cf_CloseLibrary(lib_handle);
  }

  // Try to search on non-initialized paths and files
  CFILE *file_handle = cfopen("lowercase.txt", "rb");
  EXPECT_EQ(file_handle, nullptr);
}

TEST(D3, CFileCaseSensitiveSearchNew) {
  const std::vector<std::filesystem::path> test_paths = {
      std::filesystem::path("TestDir") / "CamelCase.txt",
      std::filesystem::path("TestDir") / "lowercase.txt",
      std::filesystem::path("TestDir") / "UPPERCASE.TXT",
  };

  std::filesystem::path filename_new = cf_FindRealFileNameCaseInsensitive("no-exist-file.txt", "no-exist-dir");
  EXPECT_TRUE(filename_new.empty());

  filename_new = cf_FindRealFileNameCaseInsensitive("no-exist-file.txt");
  EXPECT_TRUE(filename_new.empty());

  auto cwd = std::filesystem::current_path();

  for (auto const &item : test_paths) {
    auto directory = cwd / item.parent_path();
    std::filesystem::path file = item.filename();
    std::string file_lc = item.filename().u8string();
    std::transform(file_lc.begin(), file_lc.end(), file_lc.begin(), ::tolower);
    std::string file_uc = item.filename().u8string();
    std::transform(file_uc.begin(), file_uc.end(), file_uc.begin(), ::toupper);

    EXPECT_FALSE(cf_FindRealFileNameCaseInsensitive(file_lc, directory).empty());
    EXPECT_FALSE(cf_FindRealFileNameCaseInsensitive(file_uc, directory).empty());

    EXPECT_FALSE(cf_FindRealFileNameCaseInsensitive(directory / file_lc).empty());
    EXPECT_FALSE(cf_FindRealFileNameCaseInsensitive(directory / file_uc).empty());

    // Now try case-insensitive path with non-existing directory in search.
    // Expected not found on case-sensitive fs.
    file_lc = item.u8string();
    std::transform(file_lc.begin(), file_lc.end(), file_lc.begin(), ::tolower);
    file_uc = item.u8string();
    std::transform(file_uc.begin(), file_uc.end(), file_uc.begin(), ::toupper);

    if (std::filesystem::is_regular_file(file_lc)) {
      EXPECT_FALSE(cf_FindRealFileNameCaseInsensitive(file_lc, cwd).empty());
      EXPECT_FALSE(cf_FindRealFileNameCaseInsensitive(file_lc).empty());
    } else {
      EXPECT_TRUE(cf_FindRealFileNameCaseInsensitive(file_lc, cwd).empty());
      EXPECT_TRUE(cf_FindRealFileNameCaseInsensitive(file_lc).empty());
    }
    if (std::filesystem::is_regular_file(file_uc)) {
      EXPECT_FALSE(cf_FindRealFileNameCaseInsensitive(file_uc, cwd).empty());
      EXPECT_FALSE(cf_FindRealFileNameCaseInsensitive(file_uc).empty());
    } else {
      EXPECT_TRUE(cf_FindRealFileNameCaseInsensitive(file_uc, cwd).empty());
      EXPECT_TRUE(cf_FindRealFileNameCaseInsensitive(file_uc).empty());
    }
  }
}

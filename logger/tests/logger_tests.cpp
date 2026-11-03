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

#include <cstdlib>
#include <filesystem>
#include <string>
#include <system_error>

#include <gtest/gtest.h>

#include "log.h"

#include "support/logger_test_files.h"

namespace fs = std::filesystem;
using namespace logger_test;

namespace {

// InitLog sets up process-wide state (plog's logger singleton, function-local static
// appenders). To keep that state out of the test process, every InitLog call runs inside
// EXPECT_EXIT, i.e. in a child process.
//
// Death-test children may re-run the whole test body in a fresh process, so each test's
// scratch directory has a fixed, per-test location rather than a random one.
class InitLogDeathTest : public ::testing::Test {
protected:
  void SetUp() override {
    const ::testing::TestInfo *info = ::testing::UnitTest::GetInstance()->current_test_info();
    dir_ = fs::path(LOGGER_TESTS_SCRATCH_DIR) / info->name();
    std::error_code ec;
    fs::remove_all(dir_, ec);
    ASSERT_TRUE(fs::create_directories(dir_)) << dir_;
  }

  void TearDown() override {
    std::error_code ec;
    fs::remove_all(dir_, ec);
  }

  fs::path dir_;
};

} // namespace

// plog itself opens the file for appending, so the fresh start depends on InitLog deleting
// the old file.
TEST_F(InitLogDeathTest, FileLogReplacesLogFromPreviousRun) {
  WriteFile(LogFile(dir_), "STALE-LINE-FROM-PREVIOUS-RUN\n");

  EXPECT_EXIT(
      {
        fs::current_path(dir_);
        InitLog(plog::info, /*enable_filelog=*/true, /*enable_win_console=*/false);
        LOG_INFO << "fresh-run-marker";
        std::exit(0);
      },
      ::testing::ExitedWithCode(0), "");

  ASSERT_TRUE(fs::is_regular_file(LogFile(dir_)));
  const std::string contents = ReadFile(LogFile(dir_));
  EXPECT_TRUE(Contains(contents, "fresh-run-marker")) << contents;
  EXPECT_FALSE(Contains(contents, "STALE-LINE-FROM-PREVIOUS-RUN")) << contents;
}

TEST_F(InitLogDeathTest, FileLogHonoursLogLevel) {
  EXPECT_EXIT(
      {
        fs::current_path(dir_);
        InitLog(plog::warning, /*enable_filelog=*/true, /*enable_win_console=*/false);
        LOG_DEBUG << "debug-marker";
        LOG_INFO << "info-marker";
        LOG_WARNING << "warning-marker";
        LOG_ERROR << "error-marker";
        std::exit(0);
      },
      ::testing::ExitedWithCode(0), "");

  ASSERT_TRUE(fs::is_regular_file(LogFile(dir_)));
  const std::string contents = ReadFile(LogFile(dir_));
  EXPECT_FALSE(Contains(contents, "debug-marker")) << contents;
  EXPECT_FALSE(Contains(contents, "info-marker")) << contents;
  EXPECT_TRUE(Contains(contents, "warning-marker")) << contents;
  EXPECT_TRUE(Contains(contents, "error-marker")) << contents;
}

TEST_F(InitLogDeathTest, DisabledFileLogLeavesExistingLogFileUntouched) {
  const std::string previous = "log kept from an earlier run\n";
  WriteFile(LogFile(dir_), previous);

  EXPECT_EXIT(
      {
        fs::current_path(dir_);
        InitLog(plog::verbose, /*enable_filelog=*/false, /*enable_win_console=*/false);
        LOG_ERROR << "console-only-marker";
        std::exit(0);
      },
      ::testing::ExitedWithCode(0), "");

  ASSERT_TRUE(fs::is_regular_file(LogFile(dir_)));
  EXPECT_EQ(ReadFile(LogFile(dir_)), previous);
}

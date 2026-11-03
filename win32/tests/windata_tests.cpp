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

// Tests for oeWin32AppDatabase (win32/windata.cpp), the registry-backed
// implementation of oeAppDatabase.
//
// The class always roots itself at HKEY_CURRENT_USER, so these tests use the
// real per-user registry. To keep that equivalent to a scratch directory, every
// test works only inside a per-process key under
// HKCU\SOFTWARE\Descent3-unit-tests, which is deleted in TearDown. The default
// constructor also opens (creating it if missing) HKCU\SOFTWARE\Outrage; if
// that key did not exist before the test and is still empty, it is removed
// again so the test leaves no trace.

#include <windows.h>

#include <cstdint>
#include <cstring>
#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "AppDatabase.h"

namespace {

constexpr const char *kTestRoot = "SOFTWARE\\Descent3-unit-tests";
constexpr const char *kOutrageKey = "SOFTWARE\\Outrage";

bool KeyExists(const char *path) {
  HKEY key;
  if (RegOpenKeyExA(HKEY_CURRENT_USER, path, 0, KEY_READ, &key) != ERROR_SUCCESS)
    return false;
  RegCloseKey(key);
  return true;
}

bool KeyIsEmpty(const char *path) {
  HKEY key;
  if (RegOpenKeyExA(HKEY_CURRENT_USER, path, 0, KEY_READ, &key) != ERROR_SUCCESS)
    return false;
  DWORD subkeys = 0, values = 0;
  LONG res = RegQueryInfoKeyA(key, nullptr, nullptr, nullptr, &subkeys, nullptr, nullptr, &values, nullptr, nullptr,
                              nullptr, nullptr);
  RegCloseKey(key);
  return res == ERROR_SUCCESS && subkeys == 0 && values == 0;
}

class Win32AppDatabaseTest : public ::testing::Test {
protected:
  void SetUp() override {
    outrage_existed_ = KeyExists(kOutrageKey);
    test_path_ = std::string(kTestRoot) + "\\win32_tests-" + std::to_string(GetCurrentProcessId());
    // Clear leftovers from a previous run that crashed before TearDown.
    RegDeleteTreeA(HKEY_CURRENT_USER, test_path_.c_str());

    db_ = std::make_unique<oeWin32AppDatabase>();
    // create_record() paths are relative to the database's base key (HKCU),
    // and focus the database on the created record.
    ASSERT_TRUE(db_->create_record(test_path_.c_str()));
  }

  void TearDown() override {
    db_.reset();
    RegDeleteTreeA(HKEY_CURRENT_USER, test_path_.c_str());
    RegDeleteKeyA(HKEY_CURRENT_USER, test_path_.c_str());
    // Only succeeds if no other test process still has a key under it.
    RegDeleteKeyA(HKEY_CURRENT_USER, kTestRoot);
    if (!outrage_existed_ && KeyIsEmpty(kOutrageKey))
      RegDeleteKeyA(HKEY_CURRENT_USER, kOutrageKey);
  }

  // Writes a string the way the game's callers do (length includes the NUL).
  bool WriteString(oeAppDatabase &db, const char *label, const char *value) {
    return db.write(label, value, static_cast<int>(strlen(value) + 1));
  }

  bool outrage_existed_ = false;
  std::string test_path_;
  std::unique_ptr<oeWin32AppDatabase> db_;
};

TEST_F(Win32AppDatabaseTest, IntegerRoundTrip) {
  ASSERT_TRUE(db_->write("Positive", 123456));
  ASSERT_TRUE(db_->write("Negative", -98765));

  int positive = 0;
  int negative = 0;
  EXPECT_TRUE(db_->read_int("Positive", &positive));
  EXPECT_TRUE(db_->read_int("Negative", &negative));
  EXPECT_EQ(positive, 123456);
  EXPECT_EQ(negative, -98765);
}

// read(label, void*, wordsize) stores into an integer of the requested width.
// Values that fit the narrower type must survive, and only `wordsize` bytes of
// the destination may be written.
TEST_F(Win32AppDatabaseTest, NarrowIntegerReadsWriteOnlyTheRequestedWidth) {
  ASSERT_TRUE(db_->write("Byte", -2));
  ASSERT_TRUE(db_->write("Short", 30000));

  int8_t bytes[4] = {0, 0x5A, 0x5A, 0x5A};
  EXPECT_TRUE(db_->read_int("Byte", &bytes[0]));
  EXPECT_EQ(bytes[0], -2);
  EXPECT_EQ(bytes[1], 0x5A);
  EXPECT_EQ(bytes[2], 0x5A);
  EXPECT_EQ(bytes[3], 0x5A);

  int16_t shorts[2] = {0, 0x5A5A};
  EXPECT_TRUE(db_->read_int("Short", &shorts[0]));
  EXPECT_EQ(shorts[0], 30000);
  EXPECT_EQ(shorts[1], 0x5A5A);
}

TEST_F(Win32AppDatabaseTest, BoolReadTreatsAnyNonzeroIntegerAsTrue) {
  ASSERT_TRUE(db_->write("Zero", 0));
  ASSERT_TRUE(db_->write("One", 1));
  ASSERT_TRUE(db_->write("Seven", 7));

  bool zero = true, one = false, seven = false;
  EXPECT_TRUE(db_->read("Zero", &zero));
  EXPECT_TRUE(db_->read("One", &one));
  EXPECT_TRUE(db_->read("Seven", &seven));
  EXPECT_FALSE(zero);
  EXPECT_TRUE(one);
  EXPECT_TRUE(seven);
}

TEST_F(Win32AppDatabaseTest, StringRoundTrip) {
  const char *value = "C:\\Games\\Descent 3\\missions";
  ASSERT_TRUE(WriteString(*db_, "Path", value));

  char buf[64];
  memset(buf, 'X', sizeof(buf));
  int len = sizeof(buf);
  EXPECT_TRUE(db_->read("Path", buf, &len));
  EXPECT_STREQ(buf, value);
}

// Writing an empty string was deliberately allowed (windata.cpp revision 5).
TEST_F(Win32AppDatabaseTest, EmptyStringRoundTrip) {
  ASSERT_TRUE(WriteString(*db_, "Empty", ""));

  char buf[16];
  memset(buf, 'X', sizeof(buf));
  int len = sizeof(buf);
  EXPECT_TRUE(db_->read("Empty", buf, &len));
  EXPECT_STREQ(buf, "");
}

// A stored string longer than the caller's buffer must not be reported as a
// successful read, and nothing past the stated buffer length may be touched.
TEST_F(Win32AppDatabaseTest, StringReadIntoTooSmallBufferFailsWithoutOverrun) {
  ASSERT_TRUE(WriteString(*db_, "Long", "0123456789"));

  char buf[16];
  memset(buf, 'X', sizeof(buf));
  int len = 4;
  EXPECT_FALSE(db_->read("Long", buf, &len));
  for (size_t i = 4; i < sizeof(buf); i++)
    EXPECT_EQ(buf[i], 'X') << "byte " << i << " beyond the buffer length was written";
}

// Callers set a default and then read over it, relying on a failed read to
// leave the default in place.
TEST_F(Win32AppDatabaseTest, MissingValuesFailAndLeaveDefaultsInPlace) {
  int number = 42;
  EXPECT_FALSE(db_->read_int("DoesNotExist", &number));
  EXPECT_EQ(number, 42);

  bool flag = true;
  EXPECT_FALSE(db_->read("DoesNotExist", &flag));
  EXPECT_TRUE(flag);

  char buf[16] = "default";
  int len = sizeof(buf);
  EXPECT_FALSE(db_->read("DoesNotExist", buf, &len));
}

TEST_F(Win32AppDatabaseTest, ValuesPersistAndCanBeFoundByAnotherInstance) {
  ASSERT_TRUE(db_->write("Persisted", 77));
  ASSERT_TRUE(WriteString(*db_, "Name", "pilot"));
  db_.reset();

  oeWin32AppDatabase other;
  ASSERT_TRUE(other.lookup_record(test_path_.c_str()));

  int number = 0;
  EXPECT_TRUE(other.read_int("Persisted", &number));
  EXPECT_EQ(number, 77);

  char buf[16];
  int len = sizeof(buf);
  EXPECT_TRUE(other.read("Name", buf, &len));
  EXPECT_STREQ(buf, "pilot");
}

TEST_F(Win32AppDatabaseTest, LookupOfMissingRecordFails) {
  oeWin32AppDatabase probe;
  std::string missing = test_path_ + "\\DoesNotExist";
  EXPECT_FALSE(probe.lookup_record(missing.c_str()));
  EXPECT_FALSE(KeyExists(missing.c_str())) << "lookup_record must not create the record";
}

// A database constructed from a parent resolves record paths relative to the
// parent's current record (the "hierarchical database" use described in
// AppDatabase.h).
TEST_F(Win32AppDatabaseTest, ChildDatabaseResolvesPathsRelativeToParentRecord) {
  {
    oeWin32AppDatabase child(db_.get());
    ASSERT_TRUE(child.create_record("Child"));
    ASSERT_TRUE(child.write("Depth", 2));
  }

  std::string child_path = test_path_ + "\\Child";
  ASSERT_TRUE(KeyExists(child_path.c_str()));

  oeWin32AppDatabase verify;
  ASSERT_TRUE(verify.lookup_record(child_path.c_str()));
  int depth = 0;
  EXPECT_TRUE(verify.read_int("Depth", &depth));
  EXPECT_EQ(depth, 2);
}

} // namespace

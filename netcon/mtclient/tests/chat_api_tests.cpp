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

// Socket_connected stays 0 except while reading the user/channel lists, so
// nothing ever reaches send().

#include <cstring>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "support/chat_helpers.h"
#include "support/chat_internals.h"
#include "support/test_support.h"
#include "chat_api.h"

namespace {

using namespace mt_test;

using Names = std::vector<std::string>;

class ChatApi : public ::testing::Test {
protected:
  void SetUp() override {
    ChatInit();
    RemoveAllChatUsers();
    FlushChatCommandQueue();
    FlushChannelList();
    strcpy(Nick_name, "me");
    strcpy(Orignial_nick_name, "me");
  }
  void TearDown() override {
    RemoveAllChatUsers();
    FlushChatCommandQueue();
    FlushChannelList();
    Socket_connected = 0;
    GetChatUserList(); // frees the cached list
  }
};

TEST(ChatGetWordNum, SplitsOnAnyWhitespace) {
  const char *line = "  alpha\tbeta \r\n gamma";
  EXPECT_STREQ(GetWordNum(0, line), "alpha");
  EXPECT_STREQ(GetWordNum(1, line), "beta");
  EXPECT_STREQ(GetWordNum(2, line), "gamma");
}

TEST(ChatGetWordNum, OutOfRangeWordIsEmpty) {
  EXPECT_STREQ(GetWordNum(3, "one two"), "");
  EXPECT_STREQ(GetWordNum(0, ""), "");
  EXPECT_STREQ(GetWordNum(0, "   "), "");
}

TEST(ChatGetWordNum, ColonWordReturnsTheRestOfTheLine) {
  // IRC "trailing" parameter: everything after the ':' is one argument.
  EXPECT_STREQ(GetWordNum(2, "PRIVMSG #lobby :hello there  world"), "hello there  world");
  EXPECT_STREQ(GetWordNum(1, "PRIVMSG #lobby :hello there"), "#lobby");
}

TEST_F(ChatApi, RemoteChannelMessageShowsSenderAndFullText) {
  auto r = Remote(":alice!a@host.example PRIVMSG #lobby :hi all, how goes?");
  ASSERT_TRUE(r);
  EXPECT_NE(r->find("[alice] hi all, how goes?"), std::string::npos) << *r;
}

TEST_F(ChatApi, RemotePrivateMessageUsesFromFormat) {
  EXPECT_EQ(Remote(":bob!b@h PRIVMSG me :psst over here"), "FROM(bob): psst over here");
}

TEST_F(ChatApi, LocalChannelEchoShowsOwnNick) {
  // SendChatString echoes outgoing lines through the parser in MSG_LOCAL mode.
  auto r = Local("PRIVMSG #lobby :hello everyone");
  ASSERT_TRUE(r);
  EXPECT_NE(r->find("[me] hello everyone"), std::string::npos) << *r;
}

TEST_F(ChatApi, LocalPrivateEchoUsesToFormat) {
  EXPECT_EQ(Local("PRIVMSG bob :see you there"), "TO(bob): see you there");
}

TEST_F(ChatApi, RemoteActionIsRenderedAsEmote) {
  auto r = Remote(":alice!a@h PRIVMSG #lobby :\001ACTION waves hello\001");
  ASSERT_TRUE(r);
  EXPECT_NE(r->find("* alice waves hello"), std::string::npos) << *r;
  EXPECT_EQ(r->find('\001', 1), std::string::npos) << "CTCP delimiters must not leak into the text";
}

TEST_F(ChatApi, LocalActionIsRenderedAsEmote) {
  // This is what "/me waves" turns into before being echoed.
  auto r = Local("PRIVMSG #lobby :\001ACTION waves\001");
  ASSERT_TRUE(r);
  EXPECT_NE(r->find("* me waves"), std::string::npos) << *r;
}

TEST_F(ChatApi, MalformedLinesAreRejected) {
  EXPECT_FALSE(Remote(":prefix.only"));
  EXPECT_FALSE(Remote(""));
  EXPECT_FALSE(Remote(":x PRIVMSG #c :" + std::string(MAXLOCALSTRING, 'a')));
}

TEST_F(ChatApi, OtherUserJoiningIsAddedOnceAndAnnounced) {
  Remote(":carol!c@h JOIN :#lobby");
  EXPECT_EQ(Users(), Names({"carol"}));
  auto cmds = DrainCommands();
  EXPECT_EQ(CommandData(cmds, CC_USER_JOINING), Names({"carol"}));
  EXPECT_FALSE(HasCommand(cmds, CC_YOURCHANNEL)) << "someone else's JOIN is not our channel";
}

TEST_F(ChatApi, OwnJoinReportsTheChannelTheServerPutUsIn) {
  // Joining "#autoselect" lands us in a real channel; the client learns which from our own JOIN.
  Remote(":me!u@h JOIN :#lobby_3");
  EXPECT_EQ(CommandData(DrainCommands(), CC_YOURCHANNEL), Names({"#lobby_3"}));
}

TEST_F(ChatApi, NamesReplyAddsEveryoneWithoutModePrefixes) {
  Remote(":irc.example 353 me = #lobby :@op +voiced plain");
  EXPECT_EQ(Users(), Names({"op", "plain", "voiced"}));
}

TEST_F(ChatApi, UserNamesAreCaseInsensitive) {
  EXPECT_EQ(AddChatUser("Alice"), 1);
  EXPECT_EQ(AddChatUser("alice"), 0);
  EXPECT_EQ(Users(), Names({"Alice"}));
  char lower[] = "ALICE";
  EXPECT_EQ(RemoveChatUser(lower), 1);
  EXPECT_TRUE(Users().empty());
}

TEST_F(ChatApi, RemovingUnknownUserIsANoOp) {
  AddChatUser("alice");
  DrainCommands();
  char nobody[] = "nobody";
  EXPECT_EQ(RemoveChatUser(nobody), 0);
  EXPECT_EQ(Users(), Names({"alice"}));
  EXPECT_TRUE(DrainCommands().empty());
}

TEST_F(ChatApi, PartRemovesOnlyThatUser) {
  Remote(":irc.example 353 me = #lobby :alice bob carol");
  DrainCommands();
  Remote(":bob!b@h PART #lobby");
  EXPECT_EQ(Users(), Names({"alice", "carol"}));
  EXPECT_EQ(CommandData(DrainCommands(), CC_USER_LEAVING), Names({"bob"}));
}

TEST_F(ChatApi, OwnPartClearsTheUserList) {
  Remote(":irc.example 353 me = #lobby :me alice bob");
  Remote(":me!u@h PART #lobby");
  EXPECT_TRUE(Users().empty());
}

TEST_F(ChatApi, QuitRemovesTheUser) {
  Remote(":irc.example 353 me = #lobby :alice bob");
  Remote(":alice!a@h QUIT :Client exited");
  EXPECT_EQ(Users(), Names({"bob"}));
}

TEST_F(ChatApi, KickRemovesTheKickedUserNotTheKicker) {
  Remote(":irc.example 353 me = #lobby :me op victim");
  Remote(":op!o@h KICK #lobby victim :behave");
  EXPECT_EQ(Users(), Names({"me", "op"}));
}

TEST_F(ChatApi, BeingKickedNotifiesAndClearsTheUserList) {
  Remote(":irc.example 353 me = #lobby :me op alice");
  DrainCommands();
  Remote(":op!o@h KICK #lobby me :bye");
  EXPECT_TRUE(HasCommand(DrainCommands(), CC_KICKED));
  EXPECT_TRUE(Users().empty());
}

TEST_F(ChatApi, NickChangeIsAnnouncedAndUserListUpdated) {
  Remote(":irc.example 353 me = #lobby :alice bob");
  DrainCommands();
  Remote(":alice!a@h NICK :alicia");
  EXPECT_EQ(Users(), Names({"alicia", "bob"}));
  EXPECT_EQ(CommandData(DrainCommands(), CC_NICKCHANGED), Names({"alice alicia"}));
}

TEST_F(ChatApi, OwnNickChangeIsUsedForLaterMessages) {
  Remote(":me!u@h NICK :me_renamed");
  auto r = Local("PRIVMSG #lobby :still here");
  ASSERT_TRUE(r);
  EXPECT_NE(r->find("[me_renamed] still here"), std::string::npos) << *r;
}

TEST_F(ChatApi, EndOfMotdAdoptsTheNickTheServerAssigned) {
  // The server may have altered our nick (e.g. truncated it); 376 tells us the real one.
  Remote(":irc.example 376 me_srv :End of /MOTD command.");
  auto r = Local("PRIVMSG #lobby :hi");
  ASSERT_TRUE(r);
  EXPECT_NE(r->find("[me_srv] hi"), std::string::npos) << *r;
}

TEST_F(ChatApi, NickInUsePicksADifferentNickEachTime) {
  Remote(":irc.example 433 * me :Nickname is already in use");
  std::string first = CurrentNick();
  Remote(":irc.example 433 * me :Nickname is already in use");
  std::string second = CurrentNick();

  EXPECT_NE(first, "me");
  EXPECT_NE(second, "me");
  EXPECT_NE(first, second);
  EXPECT_EQ(first.rfind("me", 0), 0u) << "fallback should be derived from the original nick: " << first;
  EXPECT_EQ(second.rfind("me", 0), 0u) << second;
}

TEST_F(ChatApi, NickInUseRetryNickHasNoSpaces) {
  char nick[] = "Big Pilot";
  char tid[] = "1234";
  ASSERT_EQ(ConnectToChatServer("irc.invalid", 0, nick, tid), -1);
  ASSERT_STREQ(Nick_name, "Big_Pilot");

  Remote(":irc.example 433 * Big_Pilot :Nickname is already in use");
  EXPECT_EQ(strchr(Nick_name, ' '), nullptr) << "retry nick: \"" << Nick_name << "\"";
}

TEST_F(ChatApi, BadNickDisconnects) {
  EXPECT_TRUE(Remote(":irc.example 432 * b@d :Erroneous nickname"));
  EXPECT_TRUE(HasCommand(DrainCommands(), CC_DISCONNECTED));
}

TEST_F(ChatApi, WhoisReportsTrackerIdFromRealName) {
  // ConnectToChatServer registers the tracker id as the IRC real name, which
  // WHOIS returns in the 311 reply.
  EXPECT_EQ(GetTrackerIdByUser("bob"), nullptr); // request sent, waiting
  EXPECT_EQ(GetTrackerIdByUser("bob"), nullptr); // still waiting
  Remote(":irc.example 311 me bob ~user host.example * :1234567");
  const char *tid = GetTrackerIdByUser("bob");
  ASSERT_NE(tid, nullptr);
  ASSERT_NE(tid, (char *)-1);
  EXPECT_STREQ(tid, "1234567");
}

TEST_F(ChatApi, WhoisForOfflineUserReportsError) {
  GetTrackerIdByUser("ghost");
  auto r = Remote(":irc.example 401 me ghost :No such nick/channel");
  EXPECT_EQ(r, "NOTONLINE(ghost)");
  EXPECT_EQ(GetTrackerIdByUser("ghost"), (char *)-1);
}

TEST_F(ChatApi, WhoisReportsUsersChannel) {
  EXPECT_EQ(GetChannelByUser("bob"), nullptr);
  Remote(":irc.example 319 me bob :#lobby");
  Remote(":irc.example 318 me bob :End of /WHOIS list.");
  const char *chan = GetChannelByUser("bob");
  ASSERT_NE(chan, nullptr);
  ASSERT_NE(chan, (char *)-1);
  EXPECT_STREQ(chan, "#lobby");
}

TEST_F(ChatApi, WhoisWithoutChannelReportsStarEvenAfterAnEarlierLookup) {
  GetChannelByUser("bob");
  Remote(":irc.example 319 me bob :#lobby");
  Remote(":irc.example 318 me bob :End of /WHOIS list.");
  const char *chan = GetChannelByUser("bob");
  ASSERT_NE(chan, nullptr);
  ASSERT_NE(chan, (char *)-1);
  ASSERT_STREQ(chan, "#lobby");

  GetChannelByUser("carol");
  Remote(":irc.example 311 me carol ~user host.example * :7654321");
  Remote(":irc.example 318 me carol :End of /WHOIS list.");
  chan = GetChannelByUser("carol");
  ASSERT_NE(chan, nullptr);
  ASSERT_NE(chan, (char *)-1);
  EXPECT_STREQ(chan, "*");
}

TEST_F(ChatApi, ChannelListCollectsEntriesBetweenStartAndEnd) {
  Remote(":irc.example 321 me Channel :Users  Name");
  Remote(":irc.example 322 me #lobby 5 :General chat for everyone");
  Remote(":irc.example 322 me #anarchy 12 :Free for all");
  EXPECT_EQ(ChannelList(), nullptr) << "list is not available until the end marker";
  Remote(":irc.example 323 me :End of /LIST");

  const char *list = ChannelList();
  ASSERT_NE(list, nullptr);
  auto entries = ParseChannelList(list);
  std::vector<ChannelEntry> expected = {{"#lobby", 5, "General chat for everyone"}, {"#anarchy", 12, "Free for all"}};
  EXPECT_EQ(entries, expected) << list;

  EXPECT_EQ(ChannelList(), nullptr) << "a finished list is handed out once";
}

TEST_F(ChatApi, ChannelListEntriesOutsideAListingAreIgnored) {
  Remote(":irc.example 322 me #stray 1 :not requested");
  Remote(":irc.example 321 me Channel :Users  Name");
  Remote(":irc.example 322 me #lobby 3 :hello");
  Remote(":irc.example 323 me :End of /LIST");
  const char *list = ChannelList();
  ASSERT_NE(list, nullptr);
  std::vector<ChannelEntry> expected = {{"#lobby", 3, "hello"}};
  EXPECT_EQ(ParseChannelList(list), expected) << list;
}

TEST_F(ChatApi, NewListingDiscardsEntriesFromAnUnfinishedOne) {
  Remote(":irc.example 321 me Channel :Users  Name");
  Remote(":irc.example 322 me #old 1 :stale");
  Remote(":irc.example 321 me Channel :Users  Name");
  Remote(":irc.example 322 me #new 2 :fresh");
  Remote(":irc.example 323 me :End of /LIST");
  const char *list = ChannelList();
  ASSERT_NE(list, nullptr);
  std::vector<ChannelEntry> expected = {{"#new", 2, "fresh"}};
  EXPECT_EQ(ParseChannelList(list), expected) << list;
}

TEST_F(ChatApi, CommandQueueIsFirstInFirstOut) {
  AddChatCommandToQueue(CC_USER_JOINING, "a", 2);
  AddChatCommandToQueue(CC_KICKED, nullptr, 0);
  AddChatCommandToQueue(CC_USER_LEAVING, "b", 2);

  auto cmds = DrainCommands();
  ASSERT_EQ(cmds.size(), 3u);
  EXPECT_EQ(cmds[0].command, CC_USER_JOINING);
  EXPECT_EQ(cmds[0].data, "a");
  EXPECT_EQ(cmds[1].command, CC_KICKED);
  EXPECT_EQ(cmds[2].command, CC_USER_LEAVING);
  EXPECT_EQ(cmds[2].data, "b");
  EXPECT_EQ(GetChatCommandFromQueue(), nullptr);
}

} // namespace

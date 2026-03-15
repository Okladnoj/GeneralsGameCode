#pragma once

#ifdef __APPLE__
#include <vector>
#include <string>
#include <cstdint>

struct GenOnlineFriendEntry {
  int64_t userId;
  char displayName[64];
  bool online;
  char presence[64];
};

struct GenOnlineFriendList {
  std::vector<GenOnlineFriendEntry> friends;
  std::vector<GenOnlineFriendEntry> pendingRequests;
};

GenOnlineFriendList GenOnlineSocial_FetchFriends();
void GenOnlineSocial_SendFriendRequest(int64_t targetUserId);
void GenOnlineSocial_AcceptFriendRequest(int64_t targetUserId);
void GenOnlineSocial_RejectFriendRequest(int64_t targetUserId);
void GenOnlineSocial_RemoveFriend(int64_t targetUserId);

#endif

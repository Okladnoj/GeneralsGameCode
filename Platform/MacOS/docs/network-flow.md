# macOS Port — Network Flow Documentation

## Overview

This document describes the complete online networking flow for the macOS port of
C&C Generals: Zero Hour. The macOS port replicates the behavior of the Windows
community patcher **GenTool** which intercepts GameSpy SDK calls and proxies them
through the **GeneralsOnlineServices** backend.

On Windows, GenTool achieves this via **DLL hooking at runtime** (closed source).
On macOS, we do the same via **compile-time `#ifdef __APPLE__`** guards in the
game engine code.

---

## Architecture

```
┌──────────────────────────────────────────────────────────┐
│                    Game Client (macOS)                    │
│                                                          │
│  ┌──────────────┐  ┌──────────────┐  ┌────────────────┐  │
│  │ MainMenuUtils│  │ BuddyThread  │  │  PeerThread    │  │
│  │ (login flow) │  │ (emulated GP)│  │ (emulated IRC) │  │
│  └──────┬───────┘  └──────┬───────┘  └───────┬────────┘  │
│         │                 │                  │           │
│  ┌──────▼───────────────────────────────────────────┐    │
│  │          MacOSOnlineLogin.mm                      │    │
│  │          MacOSOnlineWebSocket.mm                  │    │
│  └──────────────┬─────────────────┬─────────────────┘    │
└─────────────────┼─────────────────┼──────────────────────┘
                  │ HTTPS           │ WSS
                  ▼                 ▼
┌─────────────────────────────────────────────────────────┐
│             GeneralsOnlineServices                       │
│          (api.playgenerals.online)                       │
│                                                          │
│  ┌────────────────┐  ┌──────────────┐  ┌──────────────┐  │
│  │  CheckLogin    │  │LoginWithToken│  │  WebSocket   │  │
│  │  (game code)   │  │(refresh JWT) │  │  Controller  │  │
│  └────────────────┘  └──────────────┘  └──────────────┘  │
└─────────────────────────────────────────────────────────┘
```

---

## Phase 1: Login (OAuth via Browser)

### Entry Point

`MainMenuUtils.cpp` → `startOnline()` (line ~245)

### First-Time Login

1. `GenOnline_StartLogin()` generates a random 30-char `gameCode`
2. Opens system browser at `playgenerals.online/login?gamecode=<CODE>`
3. Sets state = `WaitingForBrowser`
4. Shows cancel dialog: "Please continue in your web browser"

### Browser Side (user action)

1. User authenticates via cnc-online.net OAuth on playgenerals.online
2. Server links `gameCode` ↔ `user_id` in `pending_logins` DB table
3. Server sets pending login state = `LoginSuccess`

### Polling

`GenOnline_Update()` calls `pollCheckLogin()` every 2 seconds:

```
POST /env/live/contract/1/CheckLogin
Body: { "code": "<GAMECODE>", "client_id": "gen_online_60hz" }
```

Response states:
- `result = 0` → `Waiting` (user hasn't logged in yet, keep polling)
- `result = 1 or 2` → `LoginSuccess` (proceed)
- `result = 3` → `LoginFailed`
- HTTP 423 → user is banned

On success, response includes:
```json
{
  "result": 1,
  "session_token": "<JWT>",
  "refresh_token": "<JWT>",
  "user_id": 12345,
  "display_name": "PlayerName",
  "ws_uri": "wss://api.playgenerals.online/ws"
}
```

### Auto-Login (Saved Session)

If `GenOnline_HasSavedSession()` finds a saved `refresh_token` in
NSUserDefaults, it calls `GenOnline_TryAutoLogin()`:

```
POST /env/live/contract/1/LoginWithToken
Authorization: Bearer <refresh_token>
Body: { "client_id": "gen_online_60hz" }
```

Returns same structure as CheckLogin on success. On failure (token expired),
clears saved session and falls back to browser login.

### Source Files

| File | Role |
|------|------|
| `Platform/MacOS/Source/Main/MacOSOnlineLogin.mm` | OAuth login, polling, auto-login |
| `Platform/MacOS/Include/MacOSOnlineLogin.h` | API declarations & session struct |

---

## Phase 2: Session Setup

After `GenOnline_GetState()` returns `LoginSuccess`, `HTTPThinkWrapper()` in
`MainMenuUtils.cpp` (line ~907) processes the session:

### Step 2.1: Cancel dialog + save session

```cpp
GenOnline_CancelLogin();      // stop polling
GenOnline_SaveSession();      // persist refresh_token to NSUserDefaults
```

### Step 2.2: Connect WebSocket

```cpp
GenOnlineWS_Connect(session->wsUri, session->sessionToken);
```

WebSocket connects to `wss://api.playgenerals.online/ws` with
`Authorization: Bearer <session_token>` header.

### Step 2.3: Initialize GameSpy state

```cpp
SetUpGameSpy(nullptr, nullptr);   // create TheGameSpyInfo, message queues
TheGameSpyInfo->setLocalBaseName(session->displayName);
TheGameSpyInfo->setLocalName(session->displayName);
TheGameSpyInfo->setLocalProfileID(session->userId);
TheGameSpyInfo->setLocalEmail("");
TheGameSpyInfo->setLocalPassword("");
```

### Step 2.4: Load cached player stats

```cpp
GameSpyMiscPreferences mPref;
PSPlayerStats localPSStats = parsePlayerKVPairs(mPref.getCachedStats());
localPSStats.id = TheGameSpyInfo->getLocalProfileID();
TheGameSpyInfo->setCachedLocalPlayerStats(localPSStats);
```

### Source Files

| File | Role |
|------|------|
| `Core/GameEngine/Source/GameNetwork/GameSpy/MainMenuUtils.cpp` | Session setup orchestration |
| `Platform/MacOS/Source/Main/MacOSOnlineWebSocket.mm` | WebSocket client |
| `Platform/MacOS/Include/MacOSOnlineWebSocket.h` | WebSocket API |

---

## Phase 3: GameSpy SDK Emulation

The original game uses two GameSpy SDK threads:
- **BuddyThread** — GameSpy Presence (GP) for login/buddy list
- **PeerThread** — GameSpy Peer (IRC peerchat) for lobbies/rooms

On macOS, we skip the real SDK calls and emulate success responses.

### Step 3.1: Buddy Login (GP emulation)

`MainMenuUtils.cpp` sends `BUDDYREQUEST_LOGIN`:

```cpp
BuddyRequest buddyReq;
buddyReq.buddyRequestType = BuddyRequest::BUDDYREQUEST_LOGIN;
strlcpy(buddyReq.arg.login.nick, session->displayName, ...);
strlcpy(buddyReq.arg.login.email, "", ...);
strlcpy(buddyReq.arg.login.password, "", ...);
buddyReq.arg.login.hasFirewall = TRUE;
TheGameSpyBuddyMessageQueue->addRequest(buddyReq);
```

`BuddyThread.cpp` handles this (line ~288):

```cpp
#ifdef __APPLE__
    m_isConnected = true;
    m_isConnecting = false;
    m_profileID = TheGameSpyInfo->getLocalProfileID();

    // Emulate connectCallback → BUDDYRESPONSE_LOGIN
    BuddyResponse loginResponse;
    loginResponse.buddyResponseType = BuddyResponse::BUDDYRESPONSE_LOGIN;
    loginResponse.result = GP_NO_ERROR;
    loginResponse.profile = m_profileID;
    TheGameSpyBuddyMessageQueue->addResponse(loginResponse);

    // Trigger PEERREQUEST_LOGIN (mirrors Windows connectCallback)
    PeerRequest req;
    req.peerRequestType = PeerRequest::PEERREQUEST_LOGIN;
    req.nick = m_nick;
    req.password = m_pass;
    req.email = m_email;
    req.login.profileID = m_profileID;
    TheGameSpyPeerMessageQueue->addRequest(req);
#else
    gpConnect(con, nick, email, password, ...);  // real GP call
#endif
```

**Why email/password are empty:** On macOS, gpConnect is never called.
Authentication already happened via OAuth. The empty values are placeholders
passed through the chain but never sent anywhere.

### Step 3.2: Peer Login (IRC emulation)

`PeerThread.cpp` handles `PEERREQUEST_LOGIN` (line ~1387):

```cpp
#ifdef __APPLE__
    m_isConnected = true;
    m_isConnecting = false;
    // Skip peerConnect (no real IRC connection)
    // Skip doCDKeyAuthentication (no CD key needed)
#else
    peerConnect(peer, nick, profileID, ...);  // real IRC connection
    doCDKeyAuthentication(peer);               // CD key auth
#endif
```

### Step 3.3: Group Rooms Injection

`MainMenuUtils.cpp` injects emulated group room list (line ~967):

```cpp
// Lobby room (custom games)
PeerResponse lobbyRoom;
lobbyRoom.peerResponseType = PeerResponse::PEERRESPONSE_GROUPROOM;
lobbyRoom.groupRoom.id = 1;
lobbyRoom.groupRoomName = "Lobby";
TheGameSpyPeerMessageQueue->addResponse(lobbyRoom);

// Quick match room
PeerResponse qmRoom;
qmRoom.peerResponseType = PeerResponse::PEERRESPONSE_GROUPROOM;
qmRoom.groupRoom.id = 2;
qmRoom.groupRoomName = "quickmatch";
TheGameSpyPeerMessageQueue->addResponse(qmRoom);

// Sentinel (end-of-list marker, id=0)
PeerResponse sentinel;
sentinel.peerResponseType = PeerResponse::PEERRESPONSE_GROUPROOM;
sentinel.groupRoom.id = 0;
TheGameSpyPeerMessageQueue->addResponse(sentinel);
```

This enables the QUICKMATCH and CUSTOM MATCH buttons in WOLWelcomeMenu.

### Step 3.4: Other Emulated Operations

| Operation | Windows (real) | macOS (emulated) |
|-----------|---------------|------------------|
| `BUDDYREQUEST_RELOGIN` | `gpConnect()` | skipped (`#ifndef __APPLE__`) |
| `BUDDYREQUEST_LOGOUT` | `gpDisconnect()` | skipped |
| `gpProcess()` | handles GP events | skipped |
| `gpIsConnected()` | checks GP state | skipped |
| `gpDestroy()` | cleans up GP | skipped |
| `PEERREQUEST_LOGOUT` | `peerDisconnect()` | skipped |
| `PEERREQUEST_JOINGROUPROOM` | `peerJoinGroupRoom()` | emulates success response |
| `PEERREQUEST_LEAVEGROUPROOM` | `peerLeaveRoom()` | emulates success response |

### Source Files

| File | Role |
|------|------|
| `Core/GameEngine/Source/GameNetwork/GameSpy/Thread/BuddyThread.cpp` | GP emulation |
| `Core/GameEngine/Source/GameNetwork/GameSpy/Thread/PeerThread.cpp` | IRC emulation |
| `Core/GameEngine/Include/GameNetwork/GameSpy/BuddyThread.h` | Request/Response types |
| `Core/GameEngine/Include/GameNetwork/GameSpy/PeerThread.h` | Request/Response types |

---

## Phase 4: Welcome Menu → Lobby

After all the above, `MainMenuUtils.cpp` pushes:

```cpp
TheShell->push("Menus/WOLWelcomeMenu.wnd");
```

WOLWelcomeMenu shows buttons: **QUICKMATCH**, **CUSTOM MATCH**, **BUDDIES**,
**RANKINGS**, etc. The buttons become enabled when the game detects that both
BuddyThread and PeerThread are "connected" and group rooms exist.

---

## Phase 5: WebSocket Protocol

All runtime communication (room lists, chat, lobbies, matchmaking, signaling)
goes through the WebSocket connection established in Phase 2.

### Message Format

All messages are JSON with an `msg_id` field:

```json
{ "msg_id": <int>, ... }
```

### Message IDs (from server)

| ID | Name | Direction |
|----|------|-----------|
| 1 | `NETWORK_ROOM_CHAT_FROM_CLIENT` | Client → Server |
| 2 | `NETWORK_ROOM_CHAT_FROM_SERVER` | Server → Client |
| 3 | `NETWORK_ROOM_CHANGE_ROOM` | Client → Server |
| 4 | `NETWORK_ROOM_MEMBER_LIST_UPDATE` | Server → Client |
| 5 | `NETWORK_ROOM_MARK_READY` | Client → Server |
| 6 | `LOBBY_CURRENT_LOBBY_UPDATE` | Server → Client |
| 7 | `NETWORK_ROOM_LOBBY_LIST_UPDATE` | Server → Client |
| 9 | `PLAYER_NAME_CHANGE` | Client → Server |
| 10 | `LOBBY_ROOM_CHAT_FROM_CLIENT` | Client → Server |
| 11 | `LOBBY_CHAT_FROM_SERVER` | Server → Client |
| 12 | `NETWORK_SIGNAL` | Bidirectional |
| 13 | `START_GAME` | Bidirectional |
| 14 | `PING` | Client → Server |
| 15 | `PONG` | Server → Client |
| 17 | `NETWORK_CONNECTION_START_SIGNALLING` | Server → Client |
| 19 | `NETWORK_CONNECTION_CLIENT_REQUEST_SIGNALLING` | Client → Server |
| 20 | `MATCHMAKING_ACTION_JOIN_PREARRANGED_LOBBY` | Server → Client |
| 21 | `MATCHMAKING_ACTION_START_GAME` | Server → Client |
| 22 | `MATCHMAKING_MESSAGE` | Server → Client |
| 23 | `START_GAME_COUNTDOWN_STARTED` | Client → Server |
| 24 | `LOBBY_REMOVE_PASSWORD` | Client → Server |
| 25 | `LOBBY_CHANGE_PASSWORD` | Client → Server |
| 26 | `FULL_MESH_CONNECTIVITY_CHECK_HOST_REQUESTS_BEGIN` | Client → Server |
| 27 | `FULL_MESH_CONNECTIVITY_CHECK_RESPONSE` | Bidirectional |
| 29 | `SOCIAL_NEW_FRIEND_REQUEST` | Server → Client |
| 30 | `SOCIAL_FRIEND_CHAT_MESSAGE_CLIENT_TO_SERVER` | Client → Server |
| 31 | `SOCIAL_FRIEND_CHAT_MESSAGE_SERVER_TO_CLIENT` | Server → Client |
| 32 | `SOCIAL_FRIEND_ONLINE_STATUS_CHANGED` | Server → Client |
| 33 | `SOCIAL_SUBSCRIBE_REALTIME_UPDATES` | Client → Server |
| 34 | `SOCIAL_UNSUBSCRIBE_REALTIME_UPDATES` | Client → Server |
| 35 | `SOCIAL_FRIENDS_OVERALL_STATUS_UPDATE` | Server → Client |
| 37 | `SOCIAL_FRIENDS_LIST_DIRTY` | Server → Client |

### Currently Implemented (macOS client)

| Feature | Status |
|---------|--------|
| PING/PONG keepalive | ✅ Implemented |
| ChangeRoom | ✅ `GenOnlineWS_SendChangeRoom()` |
| Chat (room) | ✅ `GenOnlineWS_SendChat()` |
| Receive messages | ✅ `readNextMessage()` (log only) |
| Room member list | ❌ Not wired to UI |
| Lobby list | ❌ Not wired to UI |
| Lobby creation/join | ❌ Not implemented |
| Matchmaking | ❌ Not implemented |
| P2P signaling | ❌ Not implemented |
| Friend system | ❌ Not implemented |
| Stats/Leaderboards | ❌ Not implemented |

---

## Phase 6: REST API Endpoints

Beyond WebSocket, the server exposes REST endpoints for stateless queries:

| Endpoint | Method | Purpose |
|----------|--------|---------|
| `/CheckLogin` | POST | Poll for browser login completion |
| `/LoginWithToken` | POST | Auto-login with refresh_token (JWT) |
| `/OID` | POST | Get user_id and display_name from JWT |
| `/PlayerStats` | various | Player statistics CRUD |
| `/GlobalStats` | various | Global leaderboards |
| `/Lobbies` | various | Lobby management |
| `/Rooms` | various | Room management |
| `/Friends` | various | Friend list management |
| `/MOTD` | GET | Message of the day |
| `/VersionCheck` | various | Client version validation |
| `/Matchmaking` | various | Quickmatch queue management |
| `/ServiceConfig` | various | Server configuration |

---

## Comparison: Windows GenTool vs macOS Port

| Aspect | Windows (GenTool) | macOS (built-in) |
|--------|-------------------|------------------|
| Hook mechanism | DLL injection (runtime) | `#ifdef __APPLE__` (compile-time) |
| Login UI | WOLLoginMenu shown, GenTool fills/intercepts | WOLLoginMenu skipped entirely |
| gpConnect | Intercepted at DLL level, fakes success | `#ifdef` skips call, sets connected |
| peerConnect | Intercepted at DLL level, fakes success | `#ifdef` skips call, sets connected |
| CD key auth | Intercepted, returns OK | `#ifdef` path skips entirely |
| Browser OAuth | GenTool opens browser, polls CheckLogin | MacOSOnlineLogin opens browser, polls |
| Auto-login | GenTool uses saved refresh_token | NSUserDefaults saves refresh_token |
| WebSocket | GenTool manages WS connection | MacOSOnlineWebSocket.mm manages |

---

## TODO / Next Steps

1. **Wire WebSocket to game UI** — Lobby list, room member list, chat display
2. **Implement lobby create/join** — Via REST API + WebSocket updates
3. **Implement P2P signaling** — WebRTC/ICE via WebSocket relay (msg_id 12, 17, 19)
4. **Implement friend system** — Subscribe to social updates, friend list UI
5. **Implement matchmaking** — Quickmatch queue via REST + WebSocket
6. **Implement stats reporting** — Post game results, display leaderboards
7. **Handle reconnection** — WebSocket disconnect → auto-reconnect with refresh_token

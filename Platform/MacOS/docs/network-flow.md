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
│                    Game Client (macOS)                   │
│                                                          │
│  ┌──────────────┐  ┌──────────────┐  ┌────────────────┐  │
│  │ MainMenuUtils│  │ BuddyThread  │  │  PeerThread    │  │
│  │ (login flow) │  │ (emulated GP)│  │ (emulated IRC) │  │
│  └──────┬───────┘  └──────┬───────┘  └───────┬────────┘  │
│         │                 │                  │           │
│  ┌──────▼───────────────────────────────────────────┐    │
│  │          MacOSOnlineLogin.mm                     │    │
│  │          MacOSOnlineWebSocket.mm                 │    │
│  └──────────────┬─────────────────┬─────────────────┘    │
└─────────────────┼─────────────────┼──────────────────────┘
                  │ HTTPS           │ WSS
                  ▼                 ▼
┌──────────────────────────────────────────────────────────┐
│             GeneralsOnlineServices                       │
│          (api.playgenerals.online)                       │
│                                                          │
│  ┌────────────────┐  ┌──────────────┐  ┌──────────────┐  │
│  │  CheckLogin    │  │LoginWithToken│  │  WebSocket   │  │
│  │  (game code)   │  │(refresh JWT) │  │  Controller  │  │
│  └────────────────┘  └──────────────┘  └──────────────┘  │
└──────────────────────────────────────────────────────────┘
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
| `Platform/MacOS/Source/Main/MacOSOnlineLobby.mm` | REST for rooms + lobbies |
| `Platform/MacOS/Include/MacOSOnlineLobby.h` | Lobby REST API declarations |
| `Platform/MacOS/Source/Main/MacOSOnlineWSBridge.cpp` | WS→GameSpy message injection |
| `Platform/MacOS/Include/MacOSOnlineWSBridge.h` | Bridge API declarations |

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
    // Inject PEERRESPONSE_LOGIN to signal connected state
    PeerResponse resp;
    resp.peerResponseType = PeerResponse::PEERRESPONSE_LOGIN;
    TheGameSpyPeerMessageQueue->addResponse(resp);
    // Fetch group rooms via REST
    GenOnlineLobby_FetchRooms();
#else
    peerConnect(peer, nick, profileID, ...);  // real IRC connection
    doCDKeyAuthentication(peer);               // CD key auth via registry
#endif
```

### Step 3.2.1: CD Key Authentication (skipped on macOS)

On Windows, `doCDKeyAuthentication()` reads CD key from Windows Registry
(`Software\Electronic Arts\EA Games\...\ergc`) and validates it via
`peerAuthenticateCDKey()`. On macOS:

- No Windows Registry exists → `GetStringFromRegistry()` returns `false`
- `peerAuthenticateCDKey()` is a GameSpy SDK stub → no-op
- Authentication is handled via OAuth (session_token JWT)
- CD key auth is skipped entirely with `#ifdef __APPLE__`

If not skipped, `doCDKeyAuthentication` returns `SERIAL_NONEXISTENT (0)`,
which forces `m_isConnected = false` and prevents lobby operations.

### Step 3.3: Group Rooms via REST

`GenOnlineLobby_FetchRooms()` in `MacOSOnlineLobby.mm` fetches rooms from
the server and injects them as `PEERRESPONSE_GROUPROOM` messages:

```
GET /env/live/contract/1/Rooms
Authorization: Bearer <session_token>
```

Response (from `data/rooms.json` on server):
```json
[
  { "id": 0, "name": "ALL GAMES", "flags": 1 },
  { "id": 1, "name": "General",   "flags": 0 },
  { "id": 2, "name": "1v1",       "flags": 0 },
  { "id": 3, "name": "2v2",       "flags": 0 },
  ...
]
```

Each room is injected via `GenOnlineWS_InjectGroupRoom()` in
`MacOSOnlineWSBridge.cpp`, which formats it as a `PEERRESPONSE_GROUPROOM`
message identical to what Windows receives from `listGroupRoomsCallback`.

Room 0 "ALL GAMES" is the **sentinel** — when `WOLWelcomeMenu` receives
a GROUPROOM with id=0, it calls `enableControls(TRUE)` to unlock buttons.

The call is **synchronous** (uses `dispatch_semaphore_t`) because it runs
on the PeerThread which expects blocking operations.

Receiving room id=0 enables QUICKMATCH and CUSTOM MATCH buttons in WOLWelcomeMenu.

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

### Step 4.1: Joining a Group Room

When user clicks CUSTOM MATCH, `TheGameSpyInfo->joinBestGroupRoom()` is called
(`PeerDefs.cpp` line ~378). This iterates all group rooms and picks the one
with fewest waiting players (excluding quickmatch channel).

**macOS:** `joinBestGroupRoom` forces room 0 "ALL GAMES" regardless of
the selection algorithm. Room 0 has `flags = ROOM_FLAGS_SHOW_ALL_MATCHES`,
which makes `GET /Lobbies` return all lobbies across all rooms.

The join triggers `PEERREQUEST_JOINGROUPROOM` → `PeerThread` → emulated
success → `PEERRESPONSE_JOINGROUPROOM` → WOLWelcomeMenu transitions to
`WOLCustomLobby.wnd`.

### Step 4.2: Lobby List Fetching

When WOLCustomLobby opens, it sends `PEERREQUEST_STARTGAMELIST`. On macOS,
`PeerThread` handles this by scheduling `GenOnlineLobby_FetchList()` with a
**1.5 second delay** (via `std::thread` + `sleep_for`):

```
GET /env/live/contract/1/Lobbies
Authorization: Bearer <session_token>
```

The 1.5s delay ensures the server has processed the preceding `ChangeRoom`
WebSocket message (msg_id=3) before the REST query runs.

Each lobby in the response is injected as `PEERRESPONSE_STAGINGROOM` via
`GenOnlineWS_InjectLobbyListEntry()`. After all entries are injected,
a `PEERRESPONSE_STAGINGROOMLISTCOMPLETE` sentinel is sent.

The lobby list is refreshed periodically (game sends `PEERREQUEST_STARTGAMELIST`
every few seconds while WOLCustomLobby is visible).

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
| 18 | `NETWORK_CONNECTION_DISCONNECT_PLAYER` | Server → Client |
| 26 | `FULL_MESH_CONNECTIVITY_CHECK_HOST_REQUESTS_BEGIN` | Client → Server |
| 27 | `FULL_MESH_CONNECTIVITY_CHECK_RESPONSE` | Bidirectional |
| 28 | `FULL_MESH_CONNECTIVITY_CHECK_RESPONSE_COMPLETE_TO_HOST` | Server → Host |
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
| PING/PONG keepalive | ✅ `GenOnlineWS_SendPing()` (msg_id=14→15) |
| ChangeRoom | ✅ `GenOnlineWS_SendChangeRoom()` (msg_id=3) |
| Chat (room) | ✅ Send `GenOnlineWS_SendChat()` (msg_id=1) + recv (msg_id=2) → UI |
| Chat (lobby) | ✅ Send `GenOnlineWS_SendLobbyChat()` (msg_id=10) + recv (msg_id=11) → UI |
| Room member list | ✅ Recv (msg_id=4) → `GenOnlineWS_InjectMemberJoin()` → UI |
| Lobby list | ✅ Recv (msg_id=7) → `GenOnlineWS_InjectLobbyListEntry()` → UI |
| Lobby create/join | ✅ REST `POST /Lobbies` + `PUT /Lobby/{id}` |
| Lobby update | ✅ Recv (msg_id=6) → `GenOnlineWS_InjectLobbyUpdate()` |
| Ready state | ✅ `GenOnlineWS_SendReady()` (msg_id=5) via setAccept/unAccept hooks |
| Game start | ✅ Send `GenOnlineWS_SendStartGame()` (msg_id=13) + countdown (23) |
| Lobby password | ✅ Send remove (msg_id=24) + change (msg_id=25) |
| MOTD | ✅ REST `GET /MOTD` → chat injection |
| P2P signaling (send) | ✅ Send functions (msg_id=12,19,26,27), recv→P2P state, transport hook |
| P2P disconnect | ✅ Recv (msg_id=18) → remove peer from P2P state array |
| TURN credentials | ✅ Parsed from REST response (create/join lobby) → P2P state |
| Matchmaking | ❌ Not implemented |
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
| `/Lobby/{id}` | PUT | Join lobby (returns `turn_username`, `turn_token`) |
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
| CD key auth | Registry key + peerAuthenticateCDKey | Skipped (OAuth replaces CD key) |
| Group rooms | `peerListGroupRooms()` → callback | REST `GET /Rooms` → inject |
| Lobby list | `peerStartListingGames()` → callback | REST `GET /Lobbies` → inject (1.5s delay) |
| Room joining | `peerJoinGroupRoom()` | WS `msg_id=3` + emulated success |
| Browser OAuth | GenTool opens browser, polls CheckLogin | MacOSOnlineLogin opens browser, polls |
| Auto-login | GenTool uses saved refresh_token | NSUserDefaults saves refresh_token |
| WebSocket | GenTool manages WS connection | MacOSOnlineWebSocket.mm manages |

---

## Server-Side Behavior Notes

### Lobby Filtering by Room

`LobbiesController.cs` filters lobbies by `networkRoomID`. Rooms with
`flags = ROOM_FLAGS_SHOW_ALL_MATCHES (1)` bypass this filter — server sets
`bIncludeAllNetworkRooms = true`. Only room 0 "ALL GAMES" has this flag.

For rooms with `flags = 0`, the server uses the user's `networkRoomID`
from session data. The local variable `networkRoomID` in the controller
(line ~143) is initialized to `-1` and not reassigned from session data,
so non-"ALL GAMES" rooms may return empty results.

### ChangeRoom Processing Delay

`ChangeRoom` (msg_id=3) via WebSocket is processed asynchronously.
REST `GET /Lobbies` may return stale results if called before the server
finishes processing the room change. A 1.5s delay before the REST call
accounts for this.

---

## Appendix: Join Staging Room — Full Flow Specification

### Windows Flow (reference — tested, stable)

The complete sequence from user clicking "Join" to entering the staging room:

```
[WOLLobbyMenu.cpp]
 1. CRC check: compare local exeCRC/iniCRC with roomToJoin→getExeCRC/getIniCRC
 2. Ladder check: verify ladder if applicable
 3. Full check: reject if numPlayers == MAX_SLOTS
 4. markAsStagingRoomJoiner(selectedID)
    → m_joinedStagingRoom = TRUE, m_isHosting = FALSE
    → m_localStagingRoom.reset() + enterGame()
    → find selectedID in m_stagingRooms (cached from PEERRESPONSE_STAGINGROOM)
    → GameInfoToAsciiString(info) → ParseAsciiStringToGameInfo(&m_localStagingRoom)
    → copies host/slot/map/options into m_localStagingRoom
 5. SetLobbyAttemptHostJoin(TRUE)
 6. If password → overlay; else:
 7. Send PEERREQUEST_JOINSTAGINGROOM { text=gameName, id=selectedID, password="" }

[PeerThread.cpp — PEERREQUEST_JOINSTAGINGROOM handler (Windows #else)]
 8. peerLeaveRoom(GroupRoom), peerLeaveRoom(StagingRoom)
 9. findServerByID(id) → SBServer
10. peerJoinStagingRoom(peer, server, password, joinRoomCallback, this)
    → GameSpy async call via IRC

[PeerThread.cpp — joinRoomCallback (async)]
11. Build PeerResponse PEERRESPONSE_JOINSTAGINGROOM:
    - resp.joinStagingRoom.ok = success
    - resp.joinStagingRoom.result = result (PEERJoinSuccess, PEERFullRoom, etc.)
    - resp.joinStagingRoom.isHostPresent = FALSE (initially)
    - peerEnumPlayers(StagingRoom, stagingRoomPlayerEnum, &resp)
      → for each player: stagingRoomPlayerNames[index] = nick
      → if player has PEER_FLAG_OP: isHostPresent = TRUE
12. addResponse(resp)

[WOLLobbyMenu.cpp — PEERRESPONSE_JOINSTAGINGROOM handler]
13. If resp.ok == TRUE:
    - getCurrentStagingRoom() → returns &m_localStagingRoom (because m_joinedStagingRoom=TRUE)
    - Iterate resp.stagingRoomPlayerNames[0..MAX_SLOTS-1]
    - Compare each with room→getConstSlot(0)→getName() (the host)
    - If match found → isHostPresent = TRUE
14. If ok && isHostPresent → transition to GameSpyGameOptionsMenu
15. If !ok || !isHostPresent → show error dialog, rejoin group room
```

### Staging Room Cache — Data Flow

```
[REST /Lobbies response] → GenOnlineLobby_FetchList()
  → GenOnlineWS_InjectLobbyListEntry() per lobby
    → PeerResponse::PEERRESPONSE_STAGINGROOM (PEER_ADD)
      → fills: stagingRoomPlayerNames[], profileID[], color[], faction[]
        stagingRoomMapName, stagingRoom.{id, exeCRC, iniCRC, maxPlayers, numPlayers}

[WOLLobbyMenu.cpp — PEERRESPONSE_STAGINGROOM handler]
  → Creates GameSpyStagingRoom room
  → Sets slot states from stagingRoomPlayerNames[] and profileID[]
  → Sets map from stagingRoomMapName
  → TheGameSpyInfo→addStagingRoom(room)
    → m_stagingRooms[room.getID()] = newRoom

[markAsStagingRoomJoiner(id)]
  → m_stagingRooms.find(id)
  → GameInfoToAsciiString(info) → ParseAsciiStringToGameInfo(&m_localStagingRoom)
```

### macOS Mapping

| Windows step | macOS equivalent |
|-------------|-----------------|
| `peerJoinStagingRoom()` | `GenOnlineLobby_Join()` → `PUT /Lobby/{id}` |
| `joinRoomCallback` (async) | REST completionHandler (async) |
| `peerEnumPlayers()` → `stagingRoomPlayerEnum` | Fill from `getCurrentStagingRoom()` cache |
| Response sent from callback | `GenOnlineWS_InjectJoinStagingRoomSuccess/Failure()` |
| `PEERJoinSuccess` | HTTP 200 → success |
| `PEERFullRoom` | HTTP 406 → reason=3 |
| `PEERJoinFailed` | Any other HTTP error → reason=4 |

### Key Invariants

1. `markAsStagingRoomJoiner(id)` is called BEFORE `PEERREQUEST_JOINSTAGINGROOM`
2. Staging room data must exist in `m_stagingRooms[id]` at call time
3. `PEERRESPONSE_JOINSTAGINGROOM` must NOT be sent until server confirms join
4. Response must include `stagingRoomPlayerNames[]` matching host name in slot 0
5. `getCurrentStagingRoom()` returns `&m_localStagingRoom` when `m_joinedStagingRoom=TRUE`

### Live Updates in Staging Room — Windows UTM vs macOS WS

On Windows, while in `WOLGameSetupMenu`, the host broadcasts slot data to all clients
using UTM (User-To-Many) messages:

```
Host changes anything → GameInfoToAsciiString(game) → UTM "SL" broadcast  
Client receives PEERRESPONSE_ROOMUTM "SL":
  → ParseAsciiStringToGameInfo(game, options.str())
  → WOLDisplaySlotList()
```

### SL String Format (confirmed from `GameInfoToAsciiString`, GameInfo.cpp:962)

Full format (Zero Hour, `#else` branch of `RTS_GENERALS`):
```
US=<useStats>;M=<2hexContentsMask><mapDirPath>;MC=<mapCRC_hex>;MS=<mapSize>;SD=<seed>;C=<crcInterval>;SR=<superweaponRestriction>;SC=<startingCash>;O=<oldFactionsOnly Y|N>;S=<slotList>;
```

`M=` field: first 2 characters are hex `mapContentsMask`, followed by portable map
directory path (forward slashes, no file extension, no filename — just directory).
Example: `M=3fmaps/tournament island` where `3f` = contentsMask, `maps/tournament island` = path.
Parser (`ParseAsciiStringToGameInfo`) reconstructs full path:
`maps\tournament island\tournament island.map` by duplicating last segment + adding extension.

Slot format per human player:
```
H<name>,<IP_hex>,<port>,<accepted T|F><hasMap T|F>,<color>,<playerTemplate>,<startPos>,<team>,<NATBehavior>:
```

Other slot types: `O:` (open), `X:` (closed), `C<difficulty>,<color>,<template>,<startPos>,<team>:` (AI).

Real example from runtime log:
```
US=1;M=3f! casino island v1_04 by dr_detox\! casino island v1_04 by dr_detox.map;MC=0;MS=0;SD=694860187;C=100;SR=1;SC=10000;O=N;S=Hsonmackali,0,0,TT,4,-1,-1,0,0:HDima Ok,0,0,FT,-1,-1,-1,-1,0:O:O:X:X:X;
```

### macOS SL Data Flow (confirmed)

The SL string on macOS is NOT constructed by our bridge code. It comes from the
**same `GameInfoToAsciiString()` function** used by Windows:

```
REST /Lobbies → GenOnlineLobby_FetchList()
  → GenOnlineWS_InjectLobbyListEntry() per lobby
    → PEERRESPONSE_STAGINGROOM → TheGameSpyInfo→addStagingRoom()
      → m_stagingRooms[id] cached

User clicks Join → markAsStagingRoomJoiner(id)
  → m_stagingRooms.find(id)
  → GameInfoToAsciiString(cachedRoom) → ParseAsciiStringToGameInfo(&m_localStagingRoom)
  → m_localStagingRoom populated with host/slot/map data

PUT /Lobby/{id} → HTTP 200
  → sync GET /Lobby/{id} (FetchDetails) → parse Members[]
  → InjectSlotListUTM() → PEERRESPONSE_ROOMUTM "SL" with SL string from cached data
  → InjectJoinStagingRoomSuccessWithPlayers()

WOLLobbyMenu receives JOINSTAGINGROOM:
  → checks slot0 name matches resp.stagingRoomPlayerNames[i]
  → isHostPresent=TRUE → transitions to GameSpyGameOptionsMenu

WOLGameSetupMenu receives ROOMUTM "SL":
  → isValidSlotList check: game exists, slot0 matches sender, not host → TRUE
  → ParseAsciiStringToGameInfo(game, options) → fills slots from SL string
  → WOLDisplaySlotList()
```

### ParseAsciiStringToGameInfo — Security Patch Issue

`GameInfo.cpp:1124-1132` (TheSuperHackers `@security` patch) calls
`portableMapPathToRealMapPath(mapName)`. If the map is not installed locally
(custom maps), this returns empty → the patch sets `optionsOk = FALSE` and
**rejects the entire SL string**.

On Windows, unknown maps show "map not available" in the UI but **slots still
display correctly**. The security patch is too aggressive for the join flow —
it was designed to prevent arbitrary file overwrites during map transfers, but
it also prevents slot display for any lobby with a custom map.

**Consequence:** `ParseAsciiStringToGameInfo` returns `FALSE` → `newLocalSlotNum=-1`
→ `isInGame=FALSE` → `lastSlotlistTime` never set → after 10 seconds,
`WOLGameSetupMenu` kicks player back to lobby ("Haven't seen ourselves in slotlist").

Server `LobbyMember` fields available (from `GET /Lobby/{id}` response):
- `DisplayName`, `UserID`, `Side`, `Color`, `Team`, `SlotState`, `SlotIndex`, `HasMap`


---

## TODO / Next Steps

1. ~~**Wire WebSocket to game UI**~~ — ✅ DONE (NET-09)
2. ~~**Implement lobby create/join**~~ — ✅ DONE (NET-10)
3. ~~**Implement P2P signaling**~~ — ✅ DONE (NET-11): WS send/recv, transport hook, disconnect, TURN creds
4. ~~**Fix lobby display**~~ — ✅ DONE: rooms via REST, CD key skip, room 0 workaround
5. ~~**Implement friend system**~~ — ✅ WIRED (NET-13): buddy intercepts + WS dispatch, needs runtime testing
6. ~~**Implement matchmaking**~~ — ✅ WIRED (NET-12): QM REST + WS, lobby-based flow, needs runtime testing
7. ~~**Implement stats**~~ — ✅ WIRED (NET-14): READ done, UPDATE wired, needs runtime testing
8. **Fix staging room live updates** — 🔴 NET-10.1: FetchDetails must parse Members[] and inject SL UTM
9. **Handle reconnection** — WebSocket disconnect → auto-reconnect with refresh_token
10. **Fix server networkRoomID bug** — `LobbiesController.cs` line 143 needs `networkRoomID = sourceData.networkRoomID`

---

## Confirmed Issues (2026-03-15 diagnostic session)

### Issue 1: Host Controls Disabled — NOT A BUG

`amIHost()` returns TRUE correctly when we create a lobby. Diagnostic log confirms:
```
DIAG_HOST: InitWOLGameGadgets: amIHost=1 slot0='Dima Ok' localName='Dima Ok' inGame=1
```

Controls (Limit Superweapons, Starting Cash, Limit Armies) appear disabled because
`isUsingStats=1` (Record Stats is checked). This is **correct Windows behavior** —
ranked stats games disable these controls to prevent exploits (WOLGameSetupMenu.cpp:1185):
```cpp
if (isUsingStats) {
    checkBoxLimitSuperweapons->winEnable( FALSE );
    comboBoxStartingCash->winEnable( FALSE );
    checkBoxLimitArmies->winEnable( FALSE );
}
```

### Issue 2: SL UTM Echo-Back — EXPECTED

Host sends SL UTM via WS lobby chat (`__UTM__SL:` prefix). WebSocket broadcasts to ALL
members including the sender. Host receives own SL, parses it, checks `isValidSlotList`:
```
isValidSlotList = hasGame && hasSlot0 && slot0IsPlayer && !isHost;
```

Since `isHost=1`, the host's own SL is correctly rejected (`!isHost` = FALSE).
On Windows, IRC UTM is sent only to OTHER room members, so host never sees its own SL.
Our WS echo is harmless because the `!isHost` guard filters it.

### Issue 3: Unicode Nicknames Break isPlayer() — ROOT CAUSE OF ACCEPT BUG

When joining a lobby hosted by a player with emoji/Unicode in their name:
```
DIAG_SL: slot0 isHuman=1 name='Kevin 🆅🅸🅿' sender='Kevin 🆅🅸🅿' isPlayer=0
```

Names appear identical in log but `isPlayer()` returns FALSE. This is because:

1. `resp.nick` is `std::string` (UTF-8 narrow string from WS bridge)
2. `isPlayer(AsciiString userName)` calls `uName.translate(userName)` — ASCII→UnicodeString
3. `translate()` is a byte-for-byte conversion that does NOT handle multi-byte UTF-8
4. Multi-byte emoji characters (🆅🅸🅿) get corrupted during translate
5. `GameSlot::m_name` is stored as UnicodeString (set from SL parse which also has encoding issues)
6. The corrupted representations may differ → `compareNoCase()` fails

This causes:
- `slot0IsPlayer=0` → `isValidSlotList=0` → ALL SL UTM updates are rejected
- No `UpdateSlotList()` → no `EnableAcceptControls()` → ACCEPT stays disabled
- After 10 seconds: "Haven't seen ourselves in slotlist" → kicks player

On Windows, GameSpy IRC uses ASCII-only nicknames. Emoji names only exist in the
GenOnline ecosystem where names come from playgenerals.online accounts (UTF-8).

**Fix needed**: `isValidSlotList` check must use UTF-8-aware comparison, or the SL
UTM `nick` field must match the encoding of `GameSlot::m_name`.

**Why some lobbies work**: Players with ASCII-only names (no emoji/Unicode) pass the
`isPlayer()` check correctly. Only lobbies hosted by players with Unicode names fail.

---

## Appendix: TURN Credentials Flow

TURN credentials are provided by Cloudflare via the server. They are NOT sent
over WebSocket — they arrive in the **HTTP REST response** when a lobby is
created or joined.

### Server-side flow (from `GeneralsOnlineServices`):

1. Client sends `PUT /Lobbies` (create) or `PUT /Lobby/{id}` (join)
2. Server calls `TURNCredentialManager.CreateCredentialsForUser(user_id)`
3. This calls Cloudflare API:
   `POST https://rtc.live.cloudflare.com/v1/turn/keys/{key}/credentials/generate-ice-servers`
4. Cloudflare returns ICE servers with `username` and `credential`
5. Server returns `turn_username` + `turn_token` in the HTTP response JSON
6. In DEBUG mode, server returns fake credentials (`"fake"`, `"fake"`)

### Client-side flow (macOS):

1. `GenOnlineLobby_Create()` / `GenOnlineLobby_Join()` parse JSON response
2. Extract `turn_username` and `turn_token` fields
3. Call `GenOnlineP2P_SetTURNCredentials("", username, token)` to store
4. Credentials stored in static buffers, queryable via `GenOnlineP2P_GetTURN*()`
5. Currently stored for future TURN relay fallback implementation

### When TURN relay is needed:

- If direct UDP connection between peers fails (both behind symmetric NAT)
- Client would use stored TURN credentials to relay UDP through Cloudflare
- Game's existing `Transport.cpp` UDP layer would route through TURN server
- **Not yet implemented** — direct P2P works for most LAN/same-network setups

---

## Phase 7: Quick Match (Matchmaking)

### Entry Point

`WOLQuickMatchMenu.cpp` → `ButtonStart` → `PEERREQUEST_STARTQUICKMATCH`

### Windows Legacy Flow (GameSpy MatchBot — original game)

The original Windows client joins a QM IRC channel, finds a matchbot via `peerEnumPlayers`,
sends `\CINFO` payloads (maps, side, color, NAT, CRCs), and waits in a state machine:
`QM_JOININGQMCHANNEL` → `QM_LOOKINGFORBOT` → `QM_WORKING` → `QM_MATCHED`

On `QM_MATCHED`, the UI handler (`WOLQuickMatchMenu.cpp:1318`) directly calls:
- `enterGame()`, `setSeed()`, `markGameAsQM()`, sets map, fills all slots with
  playerNames/IP/side/color/NAT from the matchbot reply, then `startGame(0)`.
- No lobby is involved. The game starts directly from matchbot data.

### Server Flow (GenTool Windows + macOS — current implementation)

The server's `MatchmakingManager` replaces the matchbot entirely.
The flow is **lobby-based**, not matchbot-based:

1. `PUT /Matchmaking` → register in queue
2. Server creates a real lobby (`LobbyManager.CreateLobby`)
3. Server sends `msg_id=20` → all clients join the lobby
4. After all joined + 5 sec countdown: server sends `msg_id=21` → start game

This means the game transitions from QM UI → staging room (lobby) → game start.
The `QM_MATCHED` handler in `WOLQuickMatchMenu.cpp` is **never used** in server flow.

**PeerThread intercepts (ifdef __APPLE__):**

| Request | REST Call | Injected Response |
|---------|-----------|-------------------|
| `PEERREQUEST_STARTQUICKMATCH` | `PUT /Matchmaking` | `QM_WORKING` or `QM_COULDNOTFINDBOT` |
| `PEERREQUEST_WIDENQUICKMATCHSEARCH` | `POST /Matchmaking/Widen` | — |
| `PEERREQUEST_STOPQUICKMATCH` | `DELETE /Matchmaking` | `QM_STOPPED` |

**WebSocket messages (server → client):**

| msg_id | Name | Action |
|--------|------|--------|
| 22 | `MATCHMAKING_MESSAGE` | Log status text (info-only, e.g. "Searching...", "2/2 players") |
| 20 | `MATCHMAKING_JOIN_LOBBY` | `GenOnlineLobby_Join(lobby_id)` — transition to staging room |
| 21 | `MATCHMAKING_START_GAME` | `GenOnlineWS_InjectGameStart()` → `PEERRESPONSE_GAMESTART` |

### Key Insight

`msg_id=21` injects `PEERRESPONSE_GAMESTART` (identical to custom match lobby start),
NOT `QM_MATCHED`. The game is already in the lobby at this point (from msg_id=20).
The staging room handler (`WOLGameSetupMenu.cpp:1741`) processes GAMESTART the same
way for both custom match and QM.

### Files

| File | Purpose |
|------|---------|
| `Platform/MacOS/Include/MacOSOnlineQM.h` | Header |
| `Platform/MacOS/Source/Main/MacOSOnlineQM.mm` | REST bridge (register, widen, deregister) |
| `Platform/MacOS/Source/Main/MacOSOnlineWSBridge.cpp` | `InjectQMStatus`, `InjectGameStart`, `InjectQMJoinLobby` |
| `Platform/MacOS/Source/Main/MacOSOnlineWebSocket.mm` | WS dispatch for msg_id 20, 21, 22 |
| `Core/.../PeerThread.cpp` | `#ifdef __APPLE__` intercepts for 3 QM requests |

---

## Ping System

### Overview

The game uses an ICMP ping system for two critical purposes:
1. **Login ping string** — latency measurement to known servers, encoded as hex and stored via `TheGameSpyInfo→setPingString()`. Gets transmitted as QR2 key `stagingServerPingString` so other players see your ping in lobby list.
2. **In-game disconnect detection** — `DisconnectManager` pings servers during stalls to determine if internet is down or if a specific player disconnected.

### Architecture (Windows)

```
PingerInterface (PingThread.h)     ← abstract interface, ThePinger global
  └── Pinger (PingThread.cpp)      ← concrete impl
        ├── addRequest(PingRequest)     ← thread-safe queue
        ├── getResponse(PingResponse)   ← thread-safe queue
        ├── getPingString(timeout)      ← hex-encoded latencies
        ├── arePingsInProgress()        ← request!=response count
        └── PingThreadClass[10]         ← 10 worker threads
              └── doPing(IP, timeout)   ← Windows: LoadLibrary("ICMP.DLL") → IcmpSendEcho
```

### Usage Points (confirmed in code)

| Location | File | What it does |
|----------|------|-------------|
| Login flow | `WOLLoginMenu.cpp:295` | `startPings()` → pings all servers from `GSConfig→getPingServers()` |
| Login completion | `WOLLoginMenu.cpp:778` | `checkLogin()` waits for pings, calls `TheGameSpyInfo→setPingString()` |
| Lobby list | `WOLLobbyMenu.cpp:1240` | `room.setPingString(resp.stagingServerPingString)` — per-room ping display |
| Join staging room | `WOLGameSetupMenu.cpp:1075` | `slot→setPingString(TheGameSpyInfo→getPingString())` — local player's ping |
| Host creates room | `WOLGameSetupMenu.cpp:1388` | `hostSlot→setPingString(TheGameSpyInfo→getPingString())` |
| SL string parse | `WOLGameSetupMenu.cpp:2182` | `slot→setPingString(token)` — ping from SL UTM update |
| In-game stall | `DisconnectManager.cpp:120-180` | Pings server every 10s during disconnect screen, tracks `pingsReceived/Sent` |

### macOS Problem

```cpp
// PingThread.cpp:425
Int PingThreadClass::doPing(UnsignedInt IP, Int timeout)
{
#ifdef _UNIX        // ← macOS defines _UNIX
   (void)IP;
   (void)timeout;
   return -1;       // ← ALWAYS returns -1!
#else
   // Windows ICMP.DLL implementation...
#endif
}
```

**Consequences:**
- `doPing()` → `-1` → `goodReps=0` → `avgPing=-1`
- `getPingString()` → hex "FF" (max timeout) for all servers
- `DisconnectManager` → `pingsReceived` never incremented → system thinks internet is dead
- Lobby list shows incorrect/max ping for all rooms

### Solution: POSIX ICMP on macOS

macOS (Darwin 19+) supports **non-privileged ICMP** via:
```c
int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
```

This does NOT require root/sudo. Replace `#ifdef _UNIX return -1` with a real
ICMP echo implementation using BSD sockets + `select()` for timeout.

### Files

| File | Role |
|------|------|
| `Core/.../PingThread.cpp:doPing()` | Needs `#ifdef __APPLE__` implementation |
| `Core/.../PingThread.cpp:Thread_Function()` | Uses `WSAStartup/WSACleanup` — needs `#ifdef` skip on macOS |
| `Core/.../DisconnectManager.cpp` | Consumer — no changes needed, uses ThePinger interface |
| `WOLLoginMenu.cpp` | Consumer — calls `startPings()`, `checkLogin()` |
| `Core/.../GSConfig.cpp` | Provides `getPingServers()` list — verify servers are reachable |



/* macOS shim: winsock.h — maps Win32 Winsock API to POSIX sockets */
#pragma once
#include "windows.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define closesocket close
#define ioctlsocket ioctl

// ── WSADATA / WSAStartup / WSACleanup (no-ops on POSIX) ───────────────
#ifndef _WSADATA_DEFINED
#define _WSADATA_DEFINED
typedef struct WSAData {
  WORD wVersion;
  WORD wHighVersion;
  char szDescription[257];
  char szSystemStatus[129];
  unsigned short iMaxSockets;
  unsigned short iMaxUdpDg;
  char *lpVendorInfo;
} WSADATA, *LPWSADATA;

inline int WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData) {
  if (lpWSAData) {
    lpWSAData->wVersion = wVersionRequested;
    lpWSAData->wHighVersion = wVersionRequested;
    lpWSAData->szDescription[0] = '\0';
    lpWSAData->szSystemStatus[0] = '\0';
    lpWSAData->iMaxSockets = 0;
    lpWSAData->iMaxUdpDg = 0;
    lpWSAData->lpVendorInfo = nullptr;
  }
  return 0;
}

inline int WSACleanup(void) { return 0; }
inline int WSAGetLastError(void) { return errno; }
#endif

// ── HOSTENT typedef ───────────────────────────────────────────────────
#ifndef _HOSTENT_DEFINED
#define _HOSTENT_DEFINED
typedef struct hostent HOSTENT;
#endif

// ── sin_addr.S_un.S_addr compatibility ────────────────────────────────
// Windows uses sin_addr.S_un.S_addr, POSIX uses sin_addr.s_addr directly.
// The game code references S_un.S_addr in a few places; this macro bridges it.
#ifndef S_un
#define S_un s_un_placeholder_do_not_use
// Make sin_addr.S_un.S_addr resolve to sin_addr.s_addr:
// We can't easily macro-patch struct member access, so we fix at call sites.
#undef S_un
#endif

// ── WSA error codes → POSIX errno values ──────────────────────────────
#ifndef WSABASEERR
#define WSABASEERR       0
#define WSAEINTR         EINTR
#define WSAEBADF         EBADF
#define WSAEACCES        EACCES
#define WSAEFAULT        EFAULT
#define WSAEINVAL        EINVAL
#define WSAEMFILE        EMFILE
#define WSAEWOULDBLOCK   EWOULDBLOCK
#define WSAEINPROGRESS   EINPROGRESS
#define WSAEALREADY      EALREADY
#define WSAENOTSOCK      ENOTSOCK
#define WSAEDESTADDRREQ  EDESTADDRREQ
#define WSAEMSGSIZE      EMSGSIZE
#define WSAEPROTOTYPE    EPROTOTYPE
#define WSAENOPROTOOPT   ENOPROTOOPT
#define WSAEPROTONOSUPPORT EPROTONOSUPPORT
#define WSAESOCKTNOSUPPORT ESOCKTNOSUPPORT
#define WSAEOPNOTSUPP    EOPNOTSUPP
#define WSAEPFNOSUPPORT  EPFNOSUPPORT
#define WSAEAFNOSUPPORT  EAFNOSUPPORT
#define WSAEADDRINUSE    EADDRINUSE
#define WSAEADDRNOTAVAIL EADDRNOTAVAIL
#define WSAENETDOWN      ENETDOWN
#define WSAENETUNREACH   ENETUNREACH
#define WSAENETRESET     ENETRESET
#define WSAECONNABORTED  ECONNABORTED
#define WSAECONNRESET    ECONNRESET
#define WSAENOBUFS       ENOBUFS
#define WSAEISCONN       EISCONN
#define WSAENOTCONN      ENOTCONN
#define WSAESHUTDOWN     ESHUTDOWN
#define WSAETOOMANYREFS  ETOOMANYREFS
#define WSAETIMEDOUT     ETIMEDOUT
#define WSAECONNREFUSED  ECONNREFUSED
#define WSAELOOP         ELOOP
#define WSAENAMETOOLONG  ENAMETOOLONG
#define WSAEHOSTDOWN     EHOSTDOWN
#define WSAEHOSTUNREACH  EHOSTUNREACH
#define WSAENOTEMPTY     ENOTEMPTY
#define WSAEPROCLIM      EPROCLIM
#define WSAEUSERS        EUSERS
#define WSAEDQUOT        EDQUOT
#define WSAESTALE        ESTALE
#define WSAEREMOTE       EREMOTE
#define WSAEDISCON       ESHUTDOWN
#define WSASYSNOTREADY   ENXIO
#define WSAVERNOTSUPPORTED ENOTSUP
#define WSANOTINITIALISED  0
#define WSAHOST_NOT_FOUND HOST_NOT_FOUND
#define WSATRY_AGAIN     TRY_AGAIN
#define WSANO_RECOVERY   NO_RECOVERY
#define WSANO_DATA       NO_DATA
#define NO_ERROR         0
#endif

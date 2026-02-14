#pragma once
#ifndef _WIN32

#include "win_compat.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef SOCKET
typedef int SOCKET;
#endif
#ifndef INVALID_SOCKET
#define INVALID_SOCKET (SOCKET)(-1)
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif

#define WSAGetLastError() errno
#define ioctlsocket ioctl

#define WSAEWOULDBLOCK EWOULDBLOCK
#define WSAEINVAL EINVAL
#define WSAEALREADY EALREADY
#define WSAEISCONN EISCONN
#define WSAECONNRESET ECONNRESET
#define WSAENOTCONN ENOTCONN

#ifndef FIONBIO
#define FIONBIO FIONBIO
#endif

// Compatibility for in_addr S_un
// This is a bit tricky. We can't easily change the system struct.
// But we can define a macro that might work if used as .S_un.S_addr
// However, most code uses it as sin_addr.S_un.S_addr.
// If we define S_un as an empty macro, it might work if we also define S_addr
// as s_addr. But S_un is a member name.

#endif

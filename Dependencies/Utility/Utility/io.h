#pragma once
#ifndef _WIN32

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define _access access
#define _open open
#define _read read
#define _write write
#define _close close
#define _lseek lseek
#define _unlink unlink

#ifndef R_OK
#define R_OK 4
#endif

#endif

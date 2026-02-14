#pragma once
#ifndef _WIN32

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define _mkdir(path) mkdir(path, 0755)
#define _getcwd getcwd
#define _chdir chdir

#endif

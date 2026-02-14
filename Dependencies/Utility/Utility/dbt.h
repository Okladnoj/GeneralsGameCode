#pragma once
#ifndef _WIN32
#include <Utility/win_compat.h>

#define DBT_DEVICEARRIVAL 0x8000
#define DBT_DEVICEQUERYREMOVE 0x8001
#define DBT_DEVICEQUERYREMOVEFAILED 0x8002
#define DBT_DEVICEREMOVEPENDING 0x8003
#define DBT_DEVICEREMOVECOMPLETE 0x8004
#define DBT_DEVICETYPESPECIFIC 0x8005
#define DBT_CUSTOMEVENT 0x8006
#define DBT_DEVTYP_VOLUME 0x00000002

typedef struct _DEV_BROADCAST_HDR {
  DWORD dbch_size;
  DWORD dbch_devicetype;
  DWORD dbch_reserved;
} DEV_BROADCAST_HDR, *PDEV_BROADCAST_HDR;

typedef struct _DEV_BROADCAST_VOLUME {
  DWORD dbcv_size;
  DWORD dbcv_devicetype;
  DWORD dbcv_reserved;
  DWORD dbcv_unitmask;
  WORD dbcv_flags;
} DEV_BROADCAST_VOLUME, *PDEV_BROADCAST_VOLUME;

#endif

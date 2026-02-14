#pragma once
#define _WIN_COMPAT_H_

#ifndef _WIN32
#ifndef _UNIX
#define _UNIX
#endif
#include <ctype.h>
#include <math.h>
#include <pthread.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <wctype.h>

typedef uint32_t u32;
typedef int32_t s32;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint8_t u8;
typedef int8_t s8;
#define __int64 long long
#define _int64 long long
typedef uint64_t __uint64;

#define _isnan isnan
#define _finite isfinite
#define _S_IFDIR S_IFDIR

#define _MCW_RC 0x00000300
#define _RC_NEAR 0x00000000
#define _RC_CHOP 0x00000300
#define _MCW_PC 0x00030000
#define _PC_24 0x00020000
#define _PC_53 0x00010000
#define _PC_64 0x00000000

inline void _fpreset() {} // NOLINT(bugprone-reserved-identifier)
inline unsigned int _statusfp() {
  return 0;
} // NOLINT(bugprone-reserved-identifier)
inline unsigned int
_controlfp(unsigned int newVal,
           unsigned int mask) { // NOLINT(bugprone-reserved-identifier)
  return 0;
}

#ifndef __max
#define __max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef __min
#define __min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef WINAPI
#define WINAPI
#endif
#ifndef APIENTRY
#define APIENTRY WINAPI
#endif
#ifndef STDAPICALLTYPE
#define STDAPICALLTYPE
#endif
#ifndef STDMETHODCALLTYPE
#define STDMETHODCALLTYPE
#endif
#ifndef CALLBACK
#define CALLBACK
#endif
#ifndef FAR
#define FAR
#endif
#ifndef NEAR
#define NEAR
#endif
#ifndef PASCAL
#define PASCAL
#endif
#ifndef STDMETHOD
#define STDMETHOD(method) virtual HRESULT STDMETHODCALLTYPE method
#endif
#ifndef STDMETHOD_
#define STDMETHOD_(type, method) virtual type STDMETHODCALLTYPE method
#endif
#ifndef STDMETHODV
#define STDMETHODV(method) virtual HRESULT STDMETHODCALLTYPE method
#endif
#ifndef STDMETHODV_
#define STDMETHODV_(type, method) virtual type STDMETHODCALLTYPE method
#endif
#ifndef PURE
#define PURE = 0
#endif
#ifndef __stdcall
#define __stdcall
#endif
#ifndef __cdecl
#define __cdecl
#endif
#ifndef __fastcall
#define __fastcall
#endif
#ifndef __declspec
#define __declspec(x)
#endif
// #ifndef interface
// #define struct struct
// #endif
#ifndef CONST
#define CONST const
#endif
#ifndef VOID
#define VOID void
#endif
#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
#ifndef OPTIONAL
#define OPTIONAL
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif

#ifndef BOOL
#if defined(__APPLE__)
#include <objc/objc.h>
#define BOOL BOOL
#elif defined(__OBJC__)
#define BOOL BOOL
#else
typedef int32_t BOOL;
#endif
#endif
typedef int32_t *PBOOL;
typedef int32_t *LPBOOL;
typedef uint16_t WORD;
typedef uint8_t BYTE;
typedef int16_t SHORT;
typedef uint32_t DWORD;
typedef uint32_t UINT;
typedef uint32_t *PUINT;
typedef uint32_t *LPUINT;
typedef int INT;
typedef float FLOAT;
typedef int32_t LONG;
typedef uint32_t ULONG;
typedef uint64_t ULONGLONG;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint8_t u8;
typedef int8_t s8;

typedef intptr_t LONG_PTR;
typedef uintptr_t ULONG_PTR;
typedef intptr_t INT_PTR;
typedef uintptr_t UINT_PTR;
typedef size_t SIZE_T;

typedef void *HANDLE;
typedef HANDLE *PHANDLE;
typedef void *HMODULE;
typedef void *HINSTANCE;
typedef void *HWND;
typedef void *HRSRC;
typedef void *HGLOBAL;
typedef void *HICON;
typedef void *HCURSOR;
typedef void *HBRUSH;
typedef void *HMENU;
typedef void *HKEY;
typedef uint16_t ATOM;
typedef void *LPVOID;
typedef void *PVOID;
typedef void *LPDLGTEMPLATE;

#define HEAP_ZERO_MEMORY 0x00000008
inline HANDLE GetProcessHeap() { return (HANDLE)1; }
inline LPVOID HeapAlloc(HANDLE hHeap, DWORD dwFlags, size_t dwBytes) {
  return malloc(dwBytes);
}
inline BOOL HeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem) {
  free(lpMem);
  return TRUE;
}
typedef const void *LPCVOID;
typedef char CHAR;
typedef char *PCHAR;
typedef char *LPSTR;
typedef const char *LPCSTR;
typedef const char *PCSTR;
typedef const char *LPCTSTR;
typedef char *LPTSTR;
typedef uint32_t *LPDWORD;
typedef int32_t *PLONG;
typedef int32_t *LPLONG;

typedef void *HIMC;
typedef void *HKL;
typedef int (*FARPROC)();
#define STDMETHODIMP HRESULT
#define STDMETHODIMP_(type) type

typedef int32_t HRESULT;
#define S_OK ((HRESULT)0L)
#define S_FALSE ((HRESULT)1L)
#define E_FAIL ((HRESULT)0x80004005L)
#define E_NOINTERFACE ((HRESULT)0x80004002L)
#define E_OUTOFMEMORY ((HRESULT)0x8007000EL)
#define E_INVALIDARG ((HRESULT)0x80070057L)
#define E_POINTER ((HRESULT)0x80004003L)
#define E_NOTIMPL ((HRESULT)0x80004001L)

#define STDMETHODIMP HRESULT
#define STDMETHODIMP_(type) type

typedef struct _GUID {
  uint32_t Data1;
  uint16_t Data2;
  uint16_t Data3;
  uint8_t Data4[8];
} GUID, IID, CLSID;

typedef GUID *LPGUID;
typedef const GUID *LPCGUID;

#define REFGUID const GUID &
#define REFIID const GUID &
#define REFCLSID const GUID &

typedef struct _SRWLOCK {
  PVOID Ptr;
} SRWLOCK, *PSRWLOCK;
#define SRWLOCK_INIT {0}

typedef struct _CONDITION_VARIABLE {
  PVOID Ptr;
} CONDITION_VARIABLE, *PCONDITION_VARIABLE;
#define CONDITION_VARIABLE_INIT {0}

#ifdef __cplusplus
inline bool operator==(const GUID &a, const GUID &b) {
  return memcmp(&a, &b, sizeof(GUID)) == 0;
}
inline bool operator!=(const GUID &a, const GUID &b) { return !(a == b); }
#endif

#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif
#endif

#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8)           \
  EXTERN_C const GUID name

#ifdef __cplusplus
extern "C" {
#endif
extern const IID IID_IUnknown;
extern const IID IID_IDispatch;
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
struct IUnknown {
  virtual HRESULT WINAPI QueryInterface(REFIID riid, void **ppvObject) = 0;
  virtual ULONG WINAPI AddRef() = 0;
  virtual ULONG WINAPI Release() = 0;
};
typedef IUnknown *LPUNKNOWN;

typedef wchar_t OLECHAR;
typedef OLECHAR *LPOLESTR;
typedef const OLECHAR *LPCOLESTR;

typedef DWORD LCID;
typedef long DISPID;
typedef void *BSTR;
typedef short VARIANT_BOOL;
typedef long SCODE;
typedef struct tagCY {
  unsigned long Lo;
  long Hi;
} CY;
typedef double DATE;
typedef unsigned short VARTYPE;

struct IDispatch;

#define S_OK ((HRESULT)0L)
#define S_FALSE ((HRESULT)1L)
#define E_FAIL ((HRESULT)0x80004005L)
#define E_INVALIDARG ((HRESULT)0x80070057L)
#define E_OUTOFMEMORY ((HRESULT)0x8007000EL)
#define E_NOTIMPL ((HRESULT)0x80004001L)
#define E_NOINTERFACE ((HRESULT)0x80004002L)
#define CLASS_E_NOAGGREGATION ((HRESULT)0x80040110L)
#define E_UNEXPECTED ((HRESULT)0x8000FFFFL)

#define MAKE_HRESULT(sev, fac, code)                                           \
  ((HRESULT)(((unsigned long)(sev) << 31) | ((unsigned long)(fac) << 16) |     \
             ((unsigned long)(code))))
#define SEVERITY_SUCCESS 0
#define SEVERITY_ERROR 1
#define FACILITY_ITF 4
#define FACILITY_TRANSIENT 21
#define REGDB_E_CLASSNOTREG ((HRESULT)0x80040154L)

static const GUID GUID_NULL = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
#define IID_NULL GUID_NULL

#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#define FAILED(hr) (((HRESULT)(hr)) < 0)

#define VT_EMPTY 0
#define VT_I4 3

#define DISPATCH_METHOD 0x1
#define DISPATCH_PROPERTYGET 0x2
#define DISPATCH_PROPERTYPUT 0x4

#define LOCALE_SYSTEM_DEFAULT 0x0800

typedef struct tagVARIANT {
  VARTYPE vt;
  WORD wReserved1;
  WORD wReserved2;
  WORD wReserved3;
  union {
    LONG lVal;
    BYTE bVal;
    SHORT iVal;
    FLOAT fltVal;
    double dblVal;
    VARIANT_BOOL boolVal;
    SCODE scode;
    CY cyVal;
    DATE date;
    BSTR bstrVal;
    IUnknown *punkVal;
    IDispatch *pdispVal;
    void *byref;
  };
} VARIANT, *LPVARIANT;

typedef struct tagDISPPARAMS {
  VARIANT *rgvarg;
  DISPID *rgdispidNamedArgs;
  UINT cArgs;
  UINT cNamedArgs;
} DISPPARAMS;

struct IDispatch : public IUnknown {
  virtual HRESULT WINAPI GetTypeInfoCount(UINT *pctinfo) = 0;
  virtual HRESULT WINAPI GetTypeInfo(UINT iTInfo, LCID lcid,
                                     void **ppTInfo) = 0;
  virtual HRESULT WINAPI GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames,
                                       UINT cNames, LCID lcid,
                                       DISPID *rgDispId) = 0;
  virtual HRESULT WINAPI Invoke(DISPID dispIdMember, REFIID riid, LCID lcid,
                                WORD wFlags, DISPPARAMS *pDispParams,
                                VARIANT *pVarResult, void *pExcepInfo,
                                UINT *puArgErr) = 0;
};
typedef IDispatch *LPDISPATCH;

struct ISequentialStream : public IUnknown {
  virtual HRESULT WINAPI Read(void *pv, ULONG cb, ULONG *pcbRead) = 0;
  virtual HRESULT WINAPI Write(const void *pv, ULONG cb, ULONG *pcbWritten) = 0;
};

struct IStream : public ISequentialStream {
  // stubs for other methods if needed
};

inline HRESULT OleInitialize(void *) { return 0; }
inline void OleUninitialize() {}
#endif

#ifndef HMONITOR_DECLARED
#define HMONITOR_DECLARED
struct HMONITOR__ {
  int unused;
};
typedef struct HMONITOR__ *HMONITOR;
#endif

#define IN
#define OUT
#define OPTIONAL

typedef int AsnInteger;
typedef int AsnInteger32;

typedef struct {
  unsigned int idLength;
  unsigned int *ids;
} AsnObjectIdentifier;

typedef struct {
  BYTE *stream;
  UINT length;
  BOOL dynamic;
} AsnOctetString;

typedef AsnOctetString AsnIPAddress;

typedef struct {
  BYTE asnType;
  union {
    AsnInteger32 number;
    AsnOctetString string;
    AsnObjectIdentifier object;
    AsnIPAddress address;
    // other types omitted as not used
  } asnValue;
} AsnAny;

typedef struct {
  AsnObjectIdentifier name;
  AsnAny value;
} RFC1157VarBind;

typedef struct {
  RFC1157VarBind *list;
  unsigned int len;
} RFC1157VarBindList;

typedef RFC1157VarBindList SnmpVarBindList;

#define SNMP_PDU_GETNEXT 1

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAKEWORD(a, b)                                                         \
  ((WORD)(((BYTE)((a) & 0xff)) | ((WORD)((BYTE)((b) & 0xff))) << 8))
#define LOBYTE(w) ((BYTE)((w) & 0xff))
#define HIBYTE(w) ((BYTE)((w) >> 8))

typedef struct WSAData {
  WORD wVersion;
  WORD wHighVersion;
  char szDescription[257];
  char szSystemStatus[129];
  unsigned short iMaxSockets;
  unsigned short iMaxUdpDg;
  char *lpVendorInfo;
} WSADATA, *LPWSADATA;

#ifndef SOCKET
typedef int SOCKET;
#endif
#ifndef INVALID_SOCKET
#define INVALID_SOCKET (SOCKET)(-1)
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif

inline int WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData) {
  if (lpWSAData) {
    memset(lpWSAData, 0, sizeof(WSADATA));
    lpWSAData->wVersion = wVersionRequested;
  }
  return 0;
}
inline int WSACleanup() { return 0; }
inline int WSAGetLastError() { return errno; }

#define WSABASEERR 10000
#define WSAEINTR EINTR
#define WSAEBADF EBADF
#define WSAEACCES EACCES
#define WSAEFAULT EFAULT
#define WSAEINVAL EINVAL
#define WSAEMFILE EMFILE
#define WSAEWOULDBLOCK EWOULDBLOCK
#define WSAEINPROGRESS EINPROGRESS
#define WSAEALREADY EALREADY
#define WSAENOTSOCK ENOTSOCK
#define WSAEDESTADDRREQ EDESTADDRREQ
#define WSAEMSGSIZE EMSGSIZE
#define WSAEPROTOTYPE EPROTOTYPE
#define WSAENOPROTOOPT ENOPROTOOPT
#define WSAEPROTONOSUPPORT EPROTONOSUPPORT
#define WSAESOCKTNOSUPPORT ESOCKTNOSUPPORT
#define WSAEOPNOTSUPP EOPNOTSUPP
#define WSAEPFNOSUPPORT EPFNOSUPPORT
#define WSAEAFNOSUPPORT EAFNOSUPPORT
#define WSAEADDRINUSE EADDRINUSE
#define WSAEADDRNOTAVAIL EADDRNOTAVAIL
#define WSAENETDOWN ENETDOWN
#define WSAENETUNREACH ENETUNREACH
#define WSAENETRESET ENETRESET
#define WSAECONNABORTED ECONNABORTED
#define WSAECONNRESET ECONNRESET
#define WSAENOBUFS ENOBUFS
#define WSAEISCONN EISCONN
#define WSAENOTCONN ENOTCONN
#define WSAESHUTDOWN ESHUTDOWN
#define WSAETOOMANYREFS ETOOMANYREFS
#define WSAETIMEDOUT ETIMEDOUT
#define WSAECONNREFUSED ECONNREFUSED
#define WSAELOOP ELOOP
#define WSAENAMETOOLONG ENAMETOOLONG
#define WSAEHOSTDOWN EHOSTDOWN
#define WSAEHOSTUNREACH EHOSTUNREACH
#define WSAENOTEMPTY ENOTEMPTY
#define WSAEPROCLIM (WSABASEERR + 67)
#define WSAEUSERS (WSABASEERR + 68)
#define WSAEDQUOT (WSABASEERR + 69)
#define WSAESTALE (WSABASEERR + 70)
#define WSAEREMOTE (WSABASEERR + 71)
#define WSAEDISCON (WSABASEERR + 101)
#define WSASYSNOTREADY (WSABASEERR + 91)
#define WSAVERNOTSUPPORTED (WSABASEERR + 92)
#define WSANOTINITIALISED (WSABASEERR + 93)
#define WSAHOST_NOT_FOUND 11001
#define WSATRY_AGAIN 11002
#define WSANO_RECOVERY 11003
#define WSANO_DATA 11004

#define GetCurrentTime GetTickCount

#define HWND_TOPMOST ((HWND) - 1)
#define HWND_NOTOPMOST ((HWND) - 2)
#define HWND_TOP ((HWND)0)
#define HWND_BOTTOM ((HWND)1)

#include "CppMacros.h"
#include "imm_compat.h"
#include "string_compat.h"
#include "thread_compat.h"
#include "time_compat.h"
#include "wchar_compat.h"

#define _P_WAIT 0
#define _P_NOWAIT 1
#define _OLD_P_OVERLAY 2
#define _P_NOWAITO 3
#define _P_DETACH 4

inline intptr_t _spawnl(int mode, const char *cmdname, const char *arg0, ...) {
  return -1; // Stub: WorldBuilder won't run anyway
}

inline HIMC ImmCreateContext() { return NULL; }
inline BOOL ImmDestroyContext(HIMC hIMC) { return 0; }
inline HIMC ImmGetContext(HWND hWnd) { return NULL; }
inline BOOL ImmReleaseContext(HWND hWnd, HIMC hIMC) { return 0; }
inline HIMC ImmAssociateContext(HWND hWnd, HIMC hIMC) { return NULL; }
inline HKL GetKeyboardLayout(DWORD idThread) { return NULL; }

/*
 * Virtual Keys, Standard Set
 */
#define VK_LBUTTON 0x01
#define VK_RBUTTON 0x02
#define VK_CANCEL 0x03
#define VK_MBUTTON 0x04 /* NOT contiguous with L \u0026 RBUTTON */

#define VK_BACK 0x08
#define VK_TAB 0x09

#define VK_CLEAR 0x0C
#define VK_RETURN 0x0D

#define VK_SHIFT 0x10
#define VK_CONTROL 0x11
#define VK_MENU 0x12
#define VK_PAUSE 0x13
#define VK_CAPITAL 0x14

#define VK_KANA 0x15
#define VK_HANGEUL 0x15 /* old name - should be here for compatibility */
#define VK_HANGUL 0x15
#define VK_JUNJA 0x17
#define VK_FINAL 0x18
#define VK_HANJA 0x19
#define VK_KANJI 0x19

#define VK_ESCAPE 0x1B

#define VK_CONVERT 0x1C
#define VK_NONCONVERT 0x1D
#define VK_ACCEPT 0x1E
#define VK_MODECHANGE 0x1F

#define VK_SPACE 0x20
#define VK_PRIOR 0x21
#define VK_NEXT 0x22
#define VK_END 0x23
#define VK_HOME 0x24
#define VK_LEFT 0x25
#define VK_UP 0x26
#define VK_RIGHT 0x27
#define VK_DOWN 0x28
#define VK_SELECT 0x29
#define VK_PRINT 0x2A
#define VK_EXECUTE 0x2B
#define VK_SNAPSHOT 0x2C
#define VK_INSERT 0x2D
#define VK_DELETE 0x2E
#define VK_HELP 0x2F

/* VK_0 thru VK_9 are the same as ASCII '0' thru '9' (0x30 - 0x39) */
/* VK_A thru VK_Z are the same as ASCII 'A' thru 'Z' (0x41 - 0x5A) */

#define VK_LWIN 0x5B
#define VK_RWIN 0x5C
#define VK_APPS 0x5D

#define VK_SLEEP 0x5F

#define VK_NUMPAD0 0x60
#define VK_NUMPAD1 0x61
#define VK_NUMPAD2 0x62
#define VK_NUMPAD3 0x63
#define VK_NUMPAD4 0x64
#define VK_NUMPAD5 0x65
#define VK_NUMPAD6 0x66
#define VK_NUMPAD7 0x67
#define VK_NUMPAD8 0x68
#define VK_NUMPAD9 0x69
#define VK_MULTIPLY 0x6A
#define VK_ADD 0x6B
#define VK_SEPARATOR 0x6C
#define VK_SUBTRACT 0x6D
#define VK_DECIMAL 0x6E
#define VK_DIVIDE 0x6F
#define VK_F1 0x70
#define VK_F2 0x71
#define VK_F3 0x72
#define VK_F4 0x73
#define VK_F5 0x74
#define VK_F6 0x75
#define VK_F7 0x76
#define VK_F8 0x77
#define VK_F9 0x78
#define VK_F10 0x79
#define VK_F11 0x7A
#define VK_F12 0x7B
#define VK_F13 0x7C
#define VK_F14 0x7D
#define VK_F15 0x7E
#define VK_F16 0x7F
#define VK_F17 0x80
#define VK_F18 0x81
#define VK_F19 0x82
#define VK_F20 0x83
#define VK_F21 0x84
#define VK_F22 0x85
#define VK_F23 0x86
#define VK_F24 0x87

#define VK_NUMLOCK 0x90
#define VK_SCROLL 0x91

/*
 * L\u0026R Control, Shift \u0026 Alt Virtual Keys[73/804] Building CXX object
 * Core/Libraries/Source/WWVegas/WWMath/CMakeFiles/core_wwmath.dir/colmathfrustum.cpp.o.
 * These are only used with GetAsyncKeyState \u0026 GetKeyState.
 * The corresponding VK_CONTROL, VK_SHIFT \u0026 VK_MENU should be used
 * with GetKeyboardState for portability.
 */
#define VK_LSHIFT 0xA0
#define VK_RSHIFT 0xA1
#define VK_LCONTROL 0xA2
#define VK_RCONTROL 0xA3
#define VK_LMENU 0xA4
#define VK_RMENU 0xA5

#define VK_PROCESSKEY 0xE5

#define VK_ATTN 0xF6
#define VK_CRSEL 0xF7
#define VK_EXSEL 0xF8
#define VK_EREOF 0xF9
#define VK_PLAY 0xFA
#define VK_ZOOM 0xFB
#define VK_NONAME 0xFC
#define VK_PA1 0xFD
#define VK_OEM_CLEAR 0xFE
typedef int32_t LONG;
typedef LONG HRESULT;
typedef int64_t LONG64;
typedef uint64_t ULONG64;
typedef uint64_t DWORD64;
typedef uintptr_t DWORD_PTR;
typedef size_t SIZE_T;

typedef struct _MEMORYSTATUS {
  DWORD dwLength;
  DWORD dwMemoryLoad;
  SIZE_T dwTotalPhys;
  SIZE_T dwAvailPhys;
  SIZE_T dwTotalPageFile;
  SIZE_T dwAvailPageFile;
  SIZE_T dwTotalVirtual;
  SIZE_T dwAvailVirtual;
} MEMORYSTATUS, *LPMEMORYSTATUS;

#define FORMAT_MESSAGE_ALLOCATE_BUFFER 0x00000100
#define FORMAT_MESSAGE_IGNORE_INSERTS 0x00000200
#define FORMAT_MESSAGE_FROM_STRING 0x00000400
#define FORMAT_MESSAGE_FROM_HMODULE 0x00000800
#define FORMAT_MESSAGE_FROM_SYSTEM 0x00001000
#define FORMAT_MESSAGE_ARGUMENT_ARRAY 0x00002000
#define FORMAT_MESSAGE_MAX_WIDTH_MASK 0x000000FF

#ifdef __cplusplus
#include <algorithm>
#endif

inline DWORD FormatMessageA(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId,
                            DWORD dwLanguageId, LPSTR lpBuffer, DWORD nSize,
                            va_list *Arguments) {
  (void)dwFlags;
  (void)lpSource;
  (void)dwLanguageId;
  (void)Arguments;
  if (lpBuffer && nSize > 0) {
    snprintf(lpBuffer, nSize, "Error 0x%08X", dwMessageId);
    return strlen(lpBuffer);
  }
  return 0;
}

#ifdef __cplusplus
template <typename T>
inline DWORD FormatMessageW(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId,
                            DWORD dwLanguageId, T *lpBuffer, DWORD nSize,
                            va_list *Arguments) {
  (void)dwFlags;
  (void)lpSource;
  (void)dwLanguageId;
  (void)Arguments;
#else
inline DWORD FormatMessageW(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId,
                            DWORD dwLanguageId, LPWSTR lpBuffer, DWORD nSize,
                            va_list *Arguments) {
#endif
  if (lpBuffer && nSize > 0) {
    // LPWSTR is char16_t* on Mac. snprintf normally works with char*.
    // This is VERY crude.
    char temp[1024];
    snprintf(temp, sizeof(temp), "Error 0x%08X", dwMessageId);
    int len_temp = (int)strlen(temp);
    int chars_to_copy = ((int)nSize - 1 < len_temp) ? (int)nSize - 1 : len_temp;
    for (int i = 0; i < chars_to_copy; ++i) {
      lpBuffer[i] = temp[i];
    }
    lpBuffer[chars_to_copy] = 0;
    return len_temp;
  }
  return 0;
}

#ifdef __cplusplus
inline int _wtoi(const WCHAR *s) {
  if (!s)
    return 0;
  int res = 0;
  while (*s >= '0' && *s <= '9') {
    res = res * 10 + (*s - '0');
    s++;
  }
  return res;
}
#ifndef _WIN32
inline int _wtoi(const wchar_t *s) {
  if (!s) {
    return 0;
  }
  return _wtoi((const WCHAR *)s);
}
#endif
#endif

#ifdef UNICODE
#define FormatMessage FormatMessageW
#else
#define FormatMessage FormatMessageA
#endif

typedef void *LPITEMIDLIST;
#define CSIDL_DESKTOPDIRECTORY 0x0010
inline HRESULT SHGetSpecialFolderLocation(HWND hwnd, int csidl,
                                          LPITEMIDLIST *ppidl) {
  if (ppidl)
    *ppidl = NULL;
  return 0; // S_OK
}
inline BOOL SHGetPathFromIDList(LPCVOID pidl, LPSTR pszPath) {
  if (pszPath)
    pszPath[0] = 0;
  return FALSE;
}

inline void GlobalMemoryStatus(LPMEMORYSTATUS lpBuffer) {
  if (lpBuffer) {
    memset(lpBuffer, 0, sizeof(MEMORYSTATUS));
    lpBuffer->dwLength = sizeof(MEMORYSTATUS);
  }
}

typedef DWORD *PDWORD;
typedef uint8_t *LPBYTE;
typedef unsigned char *PBYTE;
typedef INT_PTR LPARAM;
typedef UINT_PTR WPARAM;
typedef LONG_PTR LRESULT;
typedef void *HGLOBAL;
typedef void *HLOCAL;
#define LMEM_FIXED 0x0000
#define LMEM_ZEROINIT 0x0040
#define LPTR (LMEM_FIXED | LMEM_ZEROINIT)

typedef DWORD REGSAM;
#define HKEY_LOCAL_MACHINE ((HKEY)0x80000002)
#define HKEY_CURRENT_USER ((HKEY)0x80000001)
#define KEY_READ 0x20019
#define KEY_WRITE 0x20006
#define REG_DWORD 4
#define REG_SZ 1
#define REG_OPTION_NON_VOLATILE 0

#define CSIDL_PERSONAL 0x0005
#define CSIDL_APPDATA 0x001a

#define ERROR_ALREADY_EXISTS 183L

typedef uint32_t LCID;
#define LOCALE_USER_DEFAULT 0x0400
#define LOCALE_SYSTEM_DEFAULT 0x0800
#define DATE_SHORTDATE 0x00000001
#define TIME_NOSECONDS 0x00000002
#define TIME_FORCE24HOURFORMAT 0x00000008
#define TIME_NOTIMEMARKER 0x00000004

#define GENERIC_READ 0x80000000L
#define GENERIC_WRITE 0x40000000L
#define OPEN_EXISTING 3
#define CREATE_ALWAYS 2
#define FILE_ATTRIBUTE_NORMAL 0x00000080
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#define FILE_FLAG_WRITE_THROUGH 0x80000000
#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR) - 1)
inline void ZeroMemory(void *Destination, size_t Length) {
  memset(Destination, 0, Length);
}
#define LoadLibrary LoadLibraryA
#define ReadConsoleInput ReadConsoleInputA

#define WAIT_OBJECT_0 0
#define WAIT_TIMEOUT 258
#define WAIT_FAILED 0xFFFFFFFF
#define INFINITE 0xFFFFFFFF

#define PIPE_WAIT 0x00000000
#define PIPE_NOWAIT 0x00000001
#define PIPE_READMODE_BYTE 0x00000000
#define PIPE_READMODE_MESSAGE 0x00000002
#define PIPE_TYPE_BYTE 0x00000000
#define PIPE_TYPE_MESSAGE 0x00000004

#define FORMAT_MESSAGE_ALLOCATE_BUFFER 0x00000100
#define FORMAT_MESSAGE_IGNORE_INSERTS 0x00000200
#define FORMAT_MESSAGE_FROM_STRING 0x00000400
#define FORMAT_MESSAGE_FROM_HMODULE 0x00000800
#define FORMAT_MESSAGE_FROM_SYSTEM 0x00001000
#define FORMAT_MESSAGE_ARGUMENT_ARRAY 0x00002000
#define FORMAT_MESSAGE_MAX_WIDTH_MASK 0x000000FF

#define MB_OK 0x00000000
#define MB_OKCANCEL 0x00000001
#define MB_ABORTRETRYIGNORE 0x00000002
#define MB_YESNOCANCEL 0x00000003
#define MB_YESNO 0x00000004
#define MB_RETRYCANCEL 0x00000005
#define MB_ICONHAND 0x00000010
#define MB_ICONQUESTION 0x00000020
#define MB_ICONEXCLAMATION 0x00000030
#define MB_ICONASTERISK 0x00000040
#define MB_ICONERROR MB_ICONHAND
#define MB_ICONSTOP MB_ICONHAND
#define MB_ICONINFORMATION MB_ICONASTERISK
#define MB_ICONWARNING MB_ICONEXCLAMATION
#define MB_TASKMODAL 0x00002000
#define MB_SYSTEMMODAL 0x00001000
#define MB_APPLMODAL 0x00000000
#define MB_SETFOREGROUND 0x00010000

#define IDOK 1
#define IDCANCEL 2
#define IDABORT 3
#define IDRETRY 4
#define IDIGNORE 5
#define IDYES 6
#define IDNO 7

#define WM_DESTROY 0x0002
#define WM_MOVE 0x0003
#define WM_SIZE 0x0005
#define WM_ACTIVATE 0x0006
#define WM_SETFOCUS 0x0007
#define WM_KILLFOCUS 0x0008
#define WM_SETTEXT 0x000C
#define WM_PAINT 0x000F
#define WM_CLOSE 0x0010
#define WM_QUIT 0x0012
#define WM_ACTIVATEAPP 0x001C
#define WM_QUERYENDSESSION 0x0011
#define WM_ERASEBKGND 0x0014
#define WM_CANCELMODE 0x001F
#define WM_SETCURSOR 0x0020
#define WM_GETMINMAXINFO 0x0024
#define WM_SETFONT 0x0030
#define WM_DISPLAYCHANGE 0x007E
#define WM_NCCALCSIZE 0x0083
#define WM_NCHITTEST 0x0084
#define WM_NCPAINT 0x0085
#define WM_NCACTIVATE 0x0086
#define WM_KEYDOWN 0x0100
#define WM_KEYUP 0x0101
#define WM_CHAR 0x0102
#define WM_SYSKEYDOWN 0x0104
#define WM_SYSKEYUP 0x0105
#define WM_INITDIALOG 0x0110
#define WM_COMMAND 0x0111
#define WM_SYSCOMMAND 0x0112
#define WM_MOUSEMOVE 0x0200
#define WM_LBUTTONDOWN 0x0201
#define WM_LBUTTONUP 0x0202
#define WM_LBUTTONDBLCLK 0x0203
#define WM_RBUTTONDOWN 0x0204
#define WM_RBUTTONUP 0x0205
#define WM_RBUTTONDBLCLK 0x0206
#define WM_MBUTTONDOWN 0x0207
#define WM_MBUTTONUP 0x0208
#define WM_MBUTTONDBLCLK 0x0209
#define WM_MOUSEWHEEL 0x020A
#define WM_ENTERMENULOOP 0x0211
#define WM_EXITMENULOOP 0x0212
#define WM_POWERBROADCAST 0x0218
#define WM_DEVICECHANGE 0x0219
#define WM_ENTERSIZEMOVE 0x0231
#define WM_EXITSIZEMOVE 0x0232
#define WM_COPY 0x0301
#define WM_PASTE 0x0302
#define WM_USER 0x0400
#define WM_APP 0x8000
#define LB_ADDSTRING 0x0180

#define LOWORD(l) ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l) ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#define MAKELONG(a, b)                                                         \
  ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) |                                \
          ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))
#define MAKELPARAM(l, h) ((LPARAM)MAKELONG(l, h))

#define GWL_STYLE (-16)
#define WS_POPUP 0x80000000L
#define WS_VISIBLE 0x10000000L
#define WS_CHILD 0x40000000L
#define WS_MINIMIZE 0x20000000L
#define WS_CLIPSIBLINGS 0x04000000L
#define WS_CLIPCHILDREN 0x02000000L
#define WS_MAXIMIZE 0x01000000L
#define WS_CAPTION 0x00C00000L
#define WS_BORDER 0x00800000L
#define WS_DLGFRAME 0x00400000L
#define WS_VSCROLL 0x00200000L
#define WS_HSCROLL 0x00100000L
#define WS_SYSMENU 0x00080000L
#define WS_THICKFRAME 0x00040000L
#define WS_GROUP 0x00020000L
#define WS_TABSTOP 0x00010000L

#define WS_EX_TOPMOST 0x00000008L

#define CS_VREDRAW 0x0001
#define CS_HREDRAW 0x0002
#define CS_DBLCLKS 0x0008
#define CS_OWNDC 0x0020
#define CS_CLASSDC 0x0040
#define CS_PARENTDC 0x0080
#define CS_NOCLOSE 0x0200
#define CS_SAVEBITS 0x0800
#define CS_BYTEALIGNCLIENT 0x1000
#define CS_BYTEALIGNWINDOW 0x2000
#define CS_GLOBALCLASS 0x4000

#define MAKEINTRESOURCEA(i) ((LPSTR)((ULONG_PTR)((WORD)(i))))
#define MAKEINTRESOURCEW(i) ((LPWSTR)((ULONG_PTR)((WORD)(i))))
#ifdef UNICODE
#define MAKEINTRESOURCE MAKEINTRESOURCEW
#else
#define MAKEINTRESOURCE MAKEINTRESOURCEA
#endif

#define BLACK_BRUSH 4
#define WHITE_BRUSH 0
#define GRAY_BRUSH 2
#define DKGRAY_BRUSH 3
#define LTGRAY_BRUSH 1
#define NULL_BRUSH 5
#define HOLLOW_BRUSH NULL_BRUSH

#define SM_CXSCREEN 0
#define SM_CYSCREEN 1

#define SWP_NOSIZE 0x0001
#define SWP_NOMOVE 0x0002
#define SWP_NOZORDER 0x0004
#define SWP_NOREDRAW 0x0008
#define SWP_NOACTIVATE 0x0010
#define SWP_FRAMECHANGED 0x0020
#define SWP_SHOWWINDOW 0x0040
#define SWP_HIDEWINDOW 0x0080
#define SWP_NOCOPYBITS 0x0100
#define SWP_NOOWNERZORDER 0x0200
#define SWP_NOSENDCHANGING 0x0400
#define INVALID_FILE_ATTRIBUTES ((DWORD) - 1)

#define MONITOR_DEFAULTTONULL 0x00000000
#define MONITOR_DEFAULTTOPRIMARY 0x00000001
#define MONITOR_DEFAULTTONEAREST 0x00000002

#define EVENT_MODIFY_STATE 0x0002
#define PAGE_READWRITE 0x04
#define FILE_MAP_WRITE 0x0002

#define INFINITE 0xFFFFFFFF

#define STD_INPUT_HANDLE ((DWORD) - 10)
#define STD_OUTPUT_HANDLE ((DWORD) - 11)
#define STD_ERROR_HANDLE ((DWORD) - 12)

#define FOREGROUND_BLUE 0x0001
#define FOREGROUND_GREEN 0x0002
#define FOREGROUND_RED 0x0004
#define FOREGROUND_INTENSITY 0x0008
#define BACKGROUND_BLUE 0x0010
#define BACKGROUND_GREEN 0x0020
#define BACKGROUND_RED 0x0040
#define BACKGROUND_INTENSITY 0x0080

#define KEY_EVENT 0x0001
#define ENABLE_PROCESSED_INPUT 0x0001
#define ENABLE_LINE_INPUT 0x0002
#define ENABLE_ECHO_INPUT 0x0004
#define ENABLE_WINDOW_INPUT 0x0008
#define ENABLE_MOUSE_INPUT 0x0010

#define DECLARE_HANDLE(name)                                                   \
  struct name##__ {                                                            \
    int unused;                                                                \
  };                                                                           \
  typedef struct name##__ *name

#define SC_MAXIMIZE 0xF030
#define SC_MONITORPOWER 0xF170
#define SC_KEYMENU 0xF100
#define SC_MOVE 0xF010
#define SC_SIZE 0xF000
#define WA_INACTIVE 0

#define HTCLIENT 1

#define TEXT(x) x

DECLARE_HANDLE(HTREEITEM);

typedef struct tagRECT {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
} RECT, *PRECT, *LPRECT;
typedef const RECT *LPCRECT;

typedef struct tagPOINT {
  LONG x;
  LONG y;
} POINT, *PPOINT, *LPPOINT;

typedef void *HDC;
typedef void *HBITMAP;
typedef void *HGDIOBJ;
typedef void *HFONT;

typedef struct tagPAINTSTRUCT {
  HDC hdc;
  BOOL fErase;
  RECT rcPaint;
  BOOL fRestore;
  BOOL fIncUpdate;
  BYTE rgbReserved[32];
} PAINTSTRUCT, *PPAINTSTRUCT, *LPPAINTSTRUCT;

#define SW_HIDE 0
#define SW_SHOWNORMAL 1
#define SW_NORMAL 1
#define SW_SHOWMINIMIZED 2
#define SW_SHOWMAXIMIZED 3
#define SW_MAXIMIZE 3
#define SW_SHOWNOACTIVATE 4
#define SW_SHOW 5
#define SW_MINIMIZE 6
#define SW_SHOWMINNOACTIVE 7
#define SW_SHOWNA 8
#define SW_RESTORE 9
#define SW_SHOWDEFAULT 10
#define SW_FORCEMINIMIZE 11

#define SRCCOPY (DWORD)0x00CC0020

#ifndef HMONITOR_DECLARED
typedef void *HMONITOR;
#endif

#ifndef PURE
#define PURE = 0
#endif
#define THIS_
#define THIS void
#define DECLARE_INTERFACE(iface) struct iface
#define DECLARE_INTERFACE_(iface, baseiface) struct iface : public baseiface

#define STDAPI HRESULT
#define STDAPI_(type) type

typedef struct tagLOGFONTA {
  LONG lfHeight;
  LONG lfWidth;
  LONG lfEscapement;
  LONG lfOrientation;
  LONG lfWeight;
  BYTE lfItalic;
  BYTE lfUnderline;
  BYTE lfStrikeOut;
  BYTE lfCharSet;
  BYTE lfOutPrecision;
  BYTE lfClipPrecision;
  BYTE lfQuality;
  BYTE lfPitchAndFamily;
  CHAR lfFaceName[32];
} LOGFONTA, *PLOGFONTA, *LPLOGFONTA;
typedef LOGFONTA LOGFONT;

typedef struct _RGNDATAHEADER {
  DWORD dwSize;
  DWORD iType;
  DWORD nCount;
  DWORD nRgnSize;
  RECT rcBound;
} RGNDATAHEADER, *PRGNDATAHEADER;

typedef struct _RGNDATA {
  RGNDATAHEADER rdh;
  char Buffer[1];
} RGNDATA, *PRGNDATA, *LPRGNDATA;

typedef struct tagPALETTEENTRY {
  BYTE peRed;
  BYTE peGreen;
  BYTE peBlue;
  BYTE peFlags;
} PALETTEENTRY, *PPALETTEENTRY, *LPPALETTEENTRY;

#pragma pack(push, 1)
typedef struct tagBITMAPFILEHEADER {
  WORD bfType;
  DWORD bfSize;
  WORD bfReserved1;
  WORD bfReserved2;
  DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;
#pragma pack(pop)

typedef struct tagBITMAPINFOHEADER {
  DWORD biSize;
  LONG biWidth;
  LONG biHeight;
  WORD biPlanes;
  WORD biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG biXPelsPerMeter;
  LONG biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct tagRGBQUAD {
  BYTE rgbBlue;
  BYTE rgbGreen;
  BYTE rgbRed;
  BYTE rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPINFO {
  BITMAPINFOHEADER bmiHeader;
  RGBQUAD bmiColors[1];
} BITMAPINFO, *PBITMAPINFO;

#define BI_RGB 0L

typedef struct _COORD {
  SHORT X;
  SHORT Y;
} COORD, *PCOORD;

typedef struct _SMALL_RECT {
  SHORT Left;
  SHORT Top;
  SHORT Right;
  SHORT Bottom;
} SMALL_RECT, *PSMALL_RECT;

typedef struct _CONSOLE_SCREEN_BUFFER_INFO {
  COORD dwSize;
  COORD dwCursorPosition;
  WORD wAttributes;
  SMALL_RECT srWindow;
  COORD dwMaximumWindowSize;
} CONSOLE_SCREEN_BUFFER_INFO, *PCONSOLE_SCREEN_BUFFER_INFO;

typedef struct _CONSOLE_CURSOR_INFO {
  DWORD dwSize;
  BOOL bVisible;
} CONSOLE_CURSOR_INFO, *PCONSOLE_CURSOR_INFO;

typedef struct _CHAR_INFO {
  union {
    WCHAR UnicodeChar;
    CHAR AsciiChar;
  } Char;
  WORD Attributes;
} CHAR_INFO, *PCHAR_INFO;

typedef struct _KEY_EVENT_RECORD {
  BOOL bKeyDown;
  WORD wRepeatCount;
  WORD wVirtualKeyCode;
  WORD wVirtualScanCode;
  union {
    WCHAR UnicodeChar;
    CHAR AsciiChar;
  } uChar;
  DWORD dwControlKeyState;
} KEY_EVENT_RECORD, *PKEY_EVENT_RECORD;

typedef struct _INPUT_RECORD {
  WORD EventType;
  union {
    KEY_EVENT_RECORD KeyEvent;
  } Event;
} INPUT_RECORD, *PINPUT_RECORD;

typedef struct tagLVCOLUMNA {
  UINT mask;
  int fmt;
  int cx;
  LPSTR pszText;
  int cchTextMax;
  int iSubItem;
  int iImage;
  int iOrder;
} LVCOLUMN;
#define LVCF_TEXT 0x0004
#define LVCF_WIDTH 0x0002
#define LVCF_FMT 0x0001
#define LVCFMT_RIGHT 0x0001

typedef struct tagLVITEMA {
  UINT mask;
  int iItem;
  int iSubItem;
  UINT state;
  UINT stateMask;
  LPSTR pszText;
  int cchTextMax;
  int iImage;
  LPARAM lParam;
} LVITEM;
#define LVIF_TEXT 0x0001

typedef struct tagMONITORINFO {
  DWORD cbSize;
  RECT rcMonitor;
  RECT rcWork;
  DWORD dwFlags;
} MONITORINFO, *LPMONITORINFO;

typedef struct tagPOINTFLOAT {
  FLOAT x;
  FLOAT y;
} POINTFLOAT, *PPOINTFLOAT;

typedef struct tagGLYPHMETRICSFLOAT {
  FLOAT gmfBlackBoxX;
  FLOAT gmfBlackBoxY;
  POINTFLOAT gmfptGlyphOrigin;
  FLOAT gmfCellIncX;
  FLOAT gmfCellIncY;
} GLYPHMETRICSFLOAT, *PGLYPHMETRICSFLOAT, *LPGLYPHMETRICSFLOAT;

#define MAX_PATH 260
#define GMEM_FIXED 0x0000
#define GMEM_MOVEABLE 0x0002

#define OF_WRITE 0x0001
#define OF_CREATE 0x1000

typedef struct hostent HOSTENT;

#ifndef COLORREF
typedef uint32_t COLORREF;
#endif
#ifndef RGB
#define RGB(r, g, b)                                                           \
  ((COLORREF)(((uint8_t)(r) | ((uint16_t)((uint8_t)(g)) << 8)) |               \
              (((uint32_t)(uint8_t)(b)) << 16)))
#endif

typedef struct tagSIZE {
  int32_t cx;
  int32_t cy;
} SIZE;

typedef struct tagTEXTMETRIC {
  int32_t tmHeight;
  int32_t tmAscent;
  int32_t tmDescent;
  int32_t tmInternalLeading;
  int32_t tmExternalLeading;
  int32_t tmAveCharWidth;
  int32_t tmMaxCharWidth;
  int32_t tmWeight;
  int32_t tmOverhang;
  int32_t tmDigitizedAspectX;
  int32_t tmDigitizedAspectY;
  char tmFirstChar;
  char tmLastChar;
  char tmDefaultChar;
  char tmBreakChar;
  uint8_t tmItalic;
  uint8_t tmUnderlined;
  uint8_t tmStruckOut;
  uint8_t tmPitchAndFamily;
  uint8_t tmCharSet;
} TEXTMETRIC;

enum {
  VARIABLE_PITCH = 0x02,
  FIXED_PITCH = 0x01,
  DEFAULT_CHARSET = 1,
  ANSI_CHARSET = 0,
  OUT_DEFAULT_PRECIS = 0,
  CLIP_DEFAULT_PRECIS = 0,
  DEFAULT_QUALITY = 0,
  ANTIALIASED_QUALITY = 4,
  FF_MODERN = (4 << 4),
  FW_NORMAL = 400,
  FW_BOLD = 700
};
#define DIB_RGB_COLORS 0
#define ETO_OPAQUE 0x0002

inline HBITMAP CreateDIBSection(HDC hdc, const BITMAPINFO *pbmi, UINT usage,
                                void **ppvBits, HANDLE hSection, DWORD offset) {
  return NULL;
}
inline HDC CreateCompatibleDC(HDC hdc) { return NULL; }
inline HGDIOBJ SelectObject(HDC hdc, HGDIOBJ h) { return NULL; }
inline COLORREF SetBkColor(HDC hdc, COLORREF color) { return 0; }
inline COLORREF SetTextColor(HDC hdc, COLORREF color) { return 0; }
inline BOOL GetTextMetrics(HDC hdc, TEXTMETRIC *lptm) { return FALSE; }
inline BOOL DeleteDC(HDC hdc) { return TRUE; }
inline BOOL DeleteObject(HGDIOBJ ho) { return TRUE; }
inline BOOL ExtTextOutW(HDC hdc, int x, int y, UINT options, const RECT *lprc,
                        LPCWSTR lpString, UINT c, const int *lpDx) {
  return FALSE;
}
inline BOOL GetTextExtentPoint32W(HDC hdc, LPCWSTR lpString, int c,
                                  SIZE *lpSize) {
  return FALSE;
}

typedef struct _CRITICAL_SECTION {
  void *ptr; // Points to an allocated pthread_mutex_t
} CRITICAL_SECTION, *PCRITICAL_SECTION, *LPCRITICAL_SECTION;

typedef struct _SYSTEMTIME {
  WORD wYear;
  WORD wMonth;
  WORD wDayOfWeek;
  WORD wDay;
  WORD wHour;
  WORD wMinute;
  WORD wSecond;
  WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

typedef union _LARGE_INTEGER {
  struct {
    DWORD LowPart;
    LONG HighPart;
  };
  struct {
    DWORD LowPart;
    LONG HighPart;
  } u;
  struct {
    DWORD LowPart;
    LONG HighPart;
  } s;
  LONG64 QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

#define EXCEPTION_ACCESS_VIOLATION 0xC0000005L
#define EXCEPTION_ARRAY_BOUNDS_EXCEEDED 0xC000008CL
#define EXCEPTION_BREAKPOINT 0x80000003L
#define EXCEPTION_DATATYPE_MISALIGNMENT 0x80000002L
#define EXCEPTION_FLT_DENORMAL_OPERAND 0xC000008DL
#define EXCEPTION_FLT_DIVIDE_BY_ZERO 0xC000008EL
#define EXCEPTION_FLT_INEXACT_RESULT 0xC000008FL
#define EXCEPTION_FLT_INVALID_OPERATION 0xC0000090L
#define EXCEPTION_FLT_OVERFLOW 0xC0000091L
#define EXCEPTION_FLT_STACK_CHECK 0xC0000092L
#define EXCEPTION_FLT_UNDERFLOW 0xC0000093L
#define EXCEPTION_GUARD_PAGE 0x80000001L
#define EXCEPTION_ILLEGAL_INSTRUCTION 0xC000001DL
#define EXCEPTION_IN_PAGE_ERROR 0xC0000006L
#define EXCEPTION_INT_DIVIDE_BY_ZERO 0xC0000094L
#define EXCEPTION_INT_OVERFLOW 0xC0000095L
#define EXCEPTION_INVALID_DISPOSITION 0xC0000026L
#define EXCEPTION_NONCONTINUABLE_EXCEPTION 0xC0000025L
#define EXCEPTION_PRIV_INSTRUCTION 0xC0000096L
#define EXCEPTION_SINGLE_STEP 0x80000004L
#define EXCEPTION_STACK_OVERFLOW 0xC00000FDL
#define EXCEPTION_INVALID_HANDLE 0xC0000008L

#define EXCEPTION_CONTINUE_SEARCH 0
#define EXCEPTION_EXECUTE_HANDLER 1
#define EXCEPTION_CONTINUE_EXECUTION -1

typedef struct _EXCEPTION_RECORD {
  DWORD ExceptionCode;
  DWORD ExceptionFlags;
  struct _EXCEPTION_RECORD *ExceptionRecord;
  PVOID ExceptionAddress;
  DWORD NumberParameters;
  ULONG_PTR ExceptionInformation[15];
} EXCEPTION_RECORD, *PEXCEPTION_RECORD;

typedef struct _FLOATING_SAVE_AREA {
  DWORD ControlWord;
  DWORD StatusWord;
  DWORD TagWord;
  DWORD ErrorOffset;
  DWORD ErrorSelector;
  DWORD DataOffset;
  DWORD DataSelector;
  BYTE RegisterArea[80];
  DWORD Cr0NpxState;
} FLOATING_SAVE_AREA;

#define SIZE_OF_80387_REGISTERS 80

typedef struct _CONTEXT {
  DWORD ContextFlags;
  DWORD Dr0;
  DWORD Dr1;
  DWORD Dr2;
  DWORD Dr3;
  DWORD Dr6;
  DWORD Dr7;
  FLOATING_SAVE_AREA FloatSave;
  DWORD SegGs;
  DWORD SegFs;
  DWORD SegEs;
  DWORD SegDs;
  DWORD Edi;
  DWORD Esi;
  DWORD Ebx;
  DWORD Edx;
  DWORD Ecx;
  DWORD Eax;
  DWORD Ebp;
  DWORD Eip;
  DWORD SegCs;
  DWORD EFlags;
  DWORD Esp;
  DWORD SegSs;
  BYTE ExtendedRegisters[512];
} CONTEXT, *PCONTEXT;

#define CONTEXT_FULL 0x00010007L
#define CONTEXT_FLOATING_POINT 0x00000008L

typedef struct _EXCEPTION_POINTERS {
  PEXCEPTION_RECORD ExceptionRecord;
  PCONTEXT ContextRecord;
} EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;

#ifndef _REFGUID_DEFINED
#define REFGUID const GUID &
#define _REFGUID_DEFINED
#endif

typedef struct _FILETIME {
  DWORD dwLowDateTime;
  DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;

typedef struct _OSVERSIONINFOA {
  DWORD dwOSVersionInfoSize;
  DWORD dwMajorVersion;
  DWORD dwMinorVersion;
  DWORD dwBuildNumber;
  DWORD dwPlatformId;
  CHAR szCSDVersion[128];
} OSVERSIONINFOA, *POSVERSIONINFOA, *LPOSVERSIONINFOA;
typedef OSVERSIONINFOA OSVERSIONINFO;
typedef struct _WIN32_FIND_DATAA {
  DWORD dwFileAttributes;
  FILETIME ftCreationTime;
  FILETIME ftLastAccessTime;
  FILETIME ftLastWriteTime;
  DWORD nFileSizeHigh;
  DWORD nFileSizeLow;
  DWORD dwReserved0;
  DWORD dwReserved1;
  CHAR cFileName[260];
  CHAR cAlternateFileName[14];
} WIN32_FIND_DATAA, *PWIN32_FIND_DATAA, *LPWIN32_FIND_DATAA;
typedef WIN32_FIND_DATAA WIN32_FIND_DATA;

#define VER_PLATFORM_WIN32_NT 2
#define VER_PLATFORM_WIN32_WINDOWS 1

struct _EXCEPTION_POINTERS;
typedef LONG(WINAPI *PTOP_LEVEL_EXCEPTION_FILTER)(
    struct _EXCEPTION_POINTERS *ExceptionInfo);

#define SYMOPT_DEFERRED_LOADS 0x00000004
#define SYMOPT_LOAD_LINES 0x00000010

typedef BOOL(CALLBACK *DLGPROC)(HWND, UINT, WPARAM, LPARAM);

#ifdef __cplusplus
extern "C" {
#endif

inline void InitializeCriticalSection(LPCRITICAL_SECTION lpCS) {
  pthread_mutex_t *mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(mutex, &attr);
  pthread_mutexattr_destroy(&attr);
  lpCS->ptr = mutex;
}
inline void DeleteCriticalSection(LPCRITICAL_SECTION lpCS) {
  if (lpCS && lpCS->ptr) {
    pthread_mutex_destroy((pthread_mutex_t *)lpCS->ptr);
    free(lpCS->ptr);
    lpCS->ptr = NULL;
  }
}
inline void EnterCriticalSection(LPCRITICAL_SECTION lpCS) {
  if (lpCS && lpCS->ptr) {
    pthread_mutex_lock((pthread_mutex_t *)lpCS->ptr);
  }
}
inline void LeaveCriticalSection(LPCRITICAL_SECTION lpCS) {
  if (lpCS && lpCS->ptr) {
    pthread_mutex_unlock((pthread_mutex_t *)lpCS->ptr);
  }
}
inline BOOL TryEnterCriticalSection(LPCRITICAL_SECTION lpCS) {
  if (lpCS && lpCS->ptr) {
    return pthread_mutex_trylock((pthread_mutex_t *)lpCS->ptr) == 0;
  }
  return FALSE;
}

inline void InitCommonControls() {}
inline LRESULT SendDlgItemMessage(HWND hWnd, int nIDDlgItem, UINT Msg,
                                  WPARAM wParam, LPARAM lParam) {
  return 0;
}
inline HWND GetDlgItem(HWND hDlg, int nIDDlgItem) { return NULL; }
inline INT_PTR DialogBoxIndirect(HINSTANCE hInstance, void *lpTemplate,
                                 HWND hWndParent, DLGPROC lpDialogFunc) {
  return 0;
}
inline BOOL EndDialog(HWND hDlg, INT_PTR nResult) { return TRUE; }
inline HFONT CreateFont(int nHeight, int nWidth, int nEscapement,
                        int nOrientation, int fnWeight, DWORD fdwItalic,
                        DWORD fdwUnderline, DWORD fdwStrikeOut,
                        DWORD fdwCharSet, DWORD fdwOutputPrecision,
                        DWORD fdwClipPrecision, DWORD fdwQuality,
                        DWORD fdwPitchAndFamily, LPCSTR lpszFace) {
  return NULL;
}

inline int ListView_InsertColumn(HWND hWnd, int iCol, const LVCOLUMN *pColumn) {
  return 0;
}
inline int ListView_InsertItem(HWND hWnd, const LVITEM *pitem) { return 0; }
inline BOOL ListView_SetItem(HWND hWnd, const LVITEM *pitem) { return TRUE; }

inline BOOL GetClientRect(HWND hWnd, LPRECT lpRect) { return TRUE; }
inline BOOL AdjustWindowRect(LPRECT lpRect, DWORD dwStyle, BOOL bMenu) {
  return TRUE;
}
inline int GetSystemMetrics(int nIndex) {
  if (nIndex == SM_CXSCREEN)
    return 1024;
  if (nIndex == SM_CYSCREEN)
    return 768;
  return 0;
}
inline HCURSOR SetCursor(HCURSOR hCursor) { return NULL; }
inline HWND SetFocus(HWND hWnd) { return hWnd; }
inline void OutputDebugStringA(LPCSTR lpOutputString) {}
inline void OutputDebugStringW(LPCWSTR lpOutputString) {}
#ifdef UNICODE
#define OutputDebugString OutputDebugStringW
#else
#define OutputDebugString OutputDebugStringA
#endif

inline SHORT GetKeyState(int nVirtKey) { return 0; }
inline SHORT GetAsyncKeyState(int vKey) { return 0; }
inline BOOL GetCursorPos(LPPOINT lpPoint) {
  if (lpPoint) {
    lpPoint->x = 0;
    lpPoint->y = 0;
  }
  return TRUE;
}
inline BOOL ScreenToClient(HWND hWnd, LPPOINT lpPoint) { return TRUE; }

typedef LRESULT(CALLBACK *WNDPROC)(HWND, UINT, WPARAM, LPARAM);

typedef struct tagWNDCLASS {
  UINT style;
  WNDPROC lpfnWndProc;
  int cbClsExtra;
  int cbWndExtra;
  HINSTANCE hInstance;
  HICON hIcon;
  HCURSOR hCursor;
  HBRUSH hbrBackground;
  LPCSTR lpszMenuName;
  LPCSTR lpszClassName;
} WNDCLASS, *PWNDCLASS, *LPWNDCLASS;

inline ATOM RegisterClassA(const WNDCLASS *lpWndClass) { return 1; }
#define RegisterClass RegisterClassA

inline HICON LoadIconA(HINSTANCE hInstance, LPCSTR lpIconName) { return NULL; }
inline HICON LoadIconW(HINSTANCE hInstance, LPCWSTR lpIconName) { return NULL; }
#ifdef UNICODE
#define LoadIcon LoadIconW
#else
#define LoadIcon LoadIconA
#endif

inline HCURSOR LoadCursorA(HINSTANCE hInstance, LPCSTR lpCursorName) {
  return NULL;
}
inline HCURSOR LoadCursorW(HINSTANCE hInstance, LPCWSTR lpCursorName) {
  return NULL;
}
#ifdef UNICODE
#define LoadCursor LoadCursorW
#else
#define LoadCursor LoadCursorA
#endif

inline HCURSOR LoadCursorFromFileA(LPCSTR lpFileName) { return (HCURSOR)1; }
#define LoadCursorFromFile LoadCursorFromFileA

inline BOOL ClientToScreen(HWND hWnd, LPPOINT lpPoint) { return TRUE; }

inline BOOL ClipCursor(const RECT *lpRect) { return TRUE; }

inline HGDIOBJ GetStockObject(int fnObject) { return NULL; }

inline HWND CreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName,
                            LPCSTR lpWindowName, DWORD dwStyle, int X, int Y,
                            int nWidth, int nHeight, HWND hWndParent,
                            HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
  return (HWND)1;
}
#define CreateWindowA(lpClassName, lpWindowName, dwStyle, x, y, nWidth,        \
                      nHeight, hWndParent, hMenu, hInstance, lpParam)          \
  CreateWindowExA(0L, lpClassName, lpWindowName, dwStyle, x, y, nWidth,        \
                  nHeight, hWndParent, hMenu, hInstance, lpParam)

#ifdef UNICODE
#define CreateWindow CreateWindowW
#else
#define CreateWindow CreateWindowA
#endif
inline BOOL SetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx,
                         int cy, UINT uFlags) {
  return TRUE;
}

inline BOOL SetConsoleScreenBufferSize(HANDLE hConsoleOutput, COORD dwSize) {
  return TRUE;
}
inline BOOL SetConsoleWindowInfo(HANDLE hConsoleOutput, BOOL bAbsolute,
                                 const SMALL_RECT *lpConsoleWindow) {
  return TRUE;
}
inline BOOL WriteConsoleOutput(HANDLE hConsoleOutput, const CHAR_INFO *lpBuffer,
                               COORD dwBufferSize, COORD dwBufferCoord,
                               PSMALL_RECT lpWriteRegion) {
  return TRUE;
}
inline BOOL TerminateProcess(HANDLE hProcess, UINT uExitCode) { return TRUE; }

inline HANDLE CreateEvent(void *lpEventAttributes, BOOL bManualReset,
                          BOOL bInitialState, const char *lpName) {
  return (HANDLE)1;
}
inline BOOL SetEvent(HANDLE hEvent) { return 1; }
inline BOOL ResetEvent(HANDLE hEvent) { return 1; }
inline PTOP_LEVEL_EXCEPTION_FILTER SetUnhandledExceptionFilter(
    PTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter) {
  return (PTOP_LEVEL_EXCEPTION_FILTER)NULL;
}
inline BOOL IsBadReadPtr(const void *lp, size_t ucb) { return 0; }
inline BOOL IsBadCodePtr(FARPROC lpfn) { return FALSE; }
inline DWORD GetCurrentProcessId() { return (DWORD)getpid(); }
inline DWORD GetCurrentThreadId() {
#ifdef __APPLE__
  uint64_t tid;
  pthread_threadid_np(NULL, &tid);
  return (DWORD)tid;
#else
  return (DWORD)(uintptr_t)pthread_self();
#endif
}
inline HANDLE GetCurrentThread() { return (HANDLE)-2; }
extern char g_GeneralsCommandLine[2048];
inline LPCSTR GetCommandLineA() { return g_GeneralsCommandLine; }
inline BOOL SHGetSpecialFolderPathA(HWND hwnd, LPSTR pszPath, int csidl,
                                    BOOL fCreate) {
  const char *home = getenv("HOME");
  if (home) {
    strcpy(pszPath, home);
    return TRUE;
  }
  return FALSE;
}
#define SHGetSpecialFolderPath SHGetSpecialFolderPathA
inline BOOL GetVersionExA(LPOSVERSIONINFOA lpVersionInformation) {
  if (lpVersionInformation) {
    lpVersionInformation->dwMajorVersion = 5;
    lpVersionInformation->dwMinorVersion = 1;
    lpVersionInformation->dwPlatformId = VER_PLATFORM_WIN32_NT;
  }
  return TRUE;
}
#define GetVersionEx GetVersionExA

inline int GetDateFormatA(LCID Locale, DWORD dwFlags, const SYSTEMTIME *lpDate,
                          LPCSTR lpFormat, LPSTR lpDateStr, int cchDate) {
  if (cchDate > 0 && lpDateStr) {
    snprintf(lpDateStr, cchDate, "%04d-%02d-%02d", lpDate->wYear,
             lpDate->wMonth, lpDate->wDay);
    return (int)strlen(lpDateStr) + 1;
  }
  return 11;
}
#define GetDateFormat GetDateFormatA

inline int GetDateFormatW(LCID Locale, DWORD dwFlags, const SYSTEMTIME *lpDate,
                          LPCWSTR lpFormat, LPWSTR lpDateStr, int cchDate) {
  if (cchDate > 0 && lpDateStr) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d", lpDate->wYear, lpDate->wMonth,
             lpDate->wDay);
    for (int i = 0; i < (int)strlen(buf) + 1 && i < cchDate; i++)
      lpDateStr[i] = (WCHAR)buf[i];
    return (int)strlen(buf) + 1;
  }
  return 11;
}

inline int GetTimeFormatA(LCID Locale, DWORD dwFlags, const SYSTEMTIME *lpTime,
                          LPCSTR lpFormat, LPSTR lpTimeStr, int cchTime) {
  if (cchTime > 0 && lpTimeStr) {
    snprintf(lpTimeStr, cchTime, "%02d:%02d:%02d", lpTime->wHour,
             lpTime->wMinute, lpTime->wSecond);
    return (int)strlen(lpTimeStr) + 1;
  }
  return 9;
}
#define GetTimeFormat GetTimeFormatA

inline int GetTimeFormatW(LCID Locale, DWORD dwFlags, const SYSTEMTIME *lpTime,
                          LPCWSTR lpFormat, LPWSTR lpTimeStr, int cchTime) {
  if (cchTime > 0 && lpTimeStr) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", lpTime->wHour, lpTime->wMinute,
             lpTime->wSecond);
    for (int i = 0; i < (int)strlen(buf) + 1 && i < cchTime; i++)
      lpTimeStr[i] = (WCHAR)buf[i];
    return (int)strlen(buf) + 1;
  }
  return 9;
}

inline BOOL SetCurrentDirectoryA(LPCSTR lpPathName) {
  return chdir(lpPathName) == 0;
}
#define SetCurrentDirectory SetCurrentDirectoryA

inline HANDLE CreateMutexA(void *lpMutexAttributes, BOOL bInitialOwner,
                           LPCSTR lpName) {
  return (HANDLE)1;
}
#define CreateMutex CreateMutexA

inline BOOL ReleaseMutex(HANDLE hMutex) {
  (void)hMutex;
  return TRUE;
}

inline HANDLE OpenMutexA(DWORD dwDesiredAccess, BOOL bInheritHandle,
                         LPCSTR lpName) {
  (void)dwDesiredAccess;
  (void)bInheritHandle;
  (void)lpName;
  return (HANDLE)1;
}
#define OpenMutex OpenMutexA

inline HANDLE FindFirstFileA(LPCSTR lpFileName,
                             LPWIN32_FIND_DATAA lpFindFileData) {
  return INVALID_HANDLE_VALUE;
}
#define FindFirstFile FindFirstFileA

inline BOOL FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData) {
  return FALSE;
}
#define FindNextFile FindNextFileA

inline BOOL FindClose(HANDLE hFindFile) { return TRUE; }

inline BOOL DeleteFileA(LPCSTR lpFileName) { return remove(lpFileName) == 0; }
#define DeleteFile DeleteFileA

inline size_t wcsftime_u16(WCHAR *s, size_t maxsize, const WCHAR *format,
                           const struct tm *timeptr) {
  char fmt[128];
  for (int i = 0; format[i] && i < 127; i++)
    fmt[i] = (char)format[i];
  fmt[127] = 0;
  char buf[256];
  size_t res = strftime(buf, sizeof(buf), fmt, timeptr);
  if (res > 0) {
    size_t limit = (maxsize < res + 1) ? maxsize - 1 : res;
    for (size_t i = 0; i < limit; i++)
      s[i] = (WCHAR)buf[i];
    s[limit] = 0;
    return limit;
  }
  return 0;
}
#define wcsftime wcsftime_u16

inline DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds) {
  return WAIT_OBJECT_0;
}
inline UINT GetDoubleClickTime() { return 500; }

inline HDC GetDC(HWND hWnd) { return (HDC)NULL; }
inline int ReleaseDC(HWND hWnd, HDC hDC) { return 0; }
inline HWND GetDesktopWindow() { return (HWND)NULL; }
inline HMONITOR MonitorFromWindow(HWND hWnd, DWORD dwFlags) {
  return (HMONITOR)NULL;
}
inline BOOL GetMonitorInfo(HMONITOR hMonitor, LPMONITORINFO lpmi) {
  return TRUE;
}
inline HRSRC FindResource(HMODULE hModule, LPCSTR lpName, LPCSTR lpType) {
  return NULL;
}
inline HGLOBAL LoadResource(HMODULE hModule, HRSRC hResInfo) { return NULL; }
inline LPVOID LockResource(HGLOBAL hResData) { return NULL; }
inline DWORD SizeofResource(HMODULE hModule, HRSRC hResInfo) { return 0; }
inline void GetSystemTime(SYSTEMTIME *lpSystemTime) {
  if (lpSystemTime != NULL) {
    memset(lpSystemTime, 0, sizeof(SYSTEMTIME));
  }
}
inline BOOL GetDeviceGammaRamp(HDC hDC, LPVOID lpRamp) { return TRUE; }
inline BOOL SetDeviceGammaRamp(HDC hDC, LPVOID lpRamp) { return TRUE; }

typedef struct tagVS_FIXEDFILEINFO {
  DWORD dwSignature;
  DWORD dwStrucVersion;
  DWORD dwFileVersionMS;
  DWORD dwFileVersionLS;
  DWORD dwProductVersionMS;
  DWORD dwProductVersionLS;
  DWORD dwFileFlagsMask;
  DWORD dwFileFlags;
  DWORD dwFileOS;
  DWORD dwFileType;
  DWORD dwFileSubtype;
  DWORD dwFileDateMS;
  DWORD dwFileDateLS;
} VS_FIXEDFILEINFO;

typedef struct _IMAGE_DOS_HEADER {
  WORD e_magic;
  WORD e_cblp;
  WORD e_cp;
  WORD e_crlc;
  WORD e_cparhdr;
  WORD e_minalloc;
  WORD e_maxalloc;
  WORD e_ss;
  WORD e_sp;
  WORD e_csum;
  WORD e_ip;
  WORD e_cs;
  WORD e_lfarlc;
  WORD e_ovno;
  WORD e_res[4];
  WORD e_oemid;
  WORD e_oeminfo;
  WORD e_res2[10];
  LONG e_lfanew;
} IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

typedef struct _IMAGE_FILE_HEADER {
  WORD Machine;
  WORD NumberOfSections;
  DWORD TimeDateStamp;
  DWORD PointerToSymbolTable;
  DWORD NumberOfSymbols;
  WORD SizeOfOptionalHeader;
  WORD Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

inline DWORD GetFileVersionInfoSize(LPCSTR lptstrFilename, LPDWORD lpdwHandle) {
  return 0;
}
inline BOOL GetFileVersionInfo(LPCSTR lptstrFilename, DWORD dwHandle,
                               DWORD dwLen, LPVOID lpData) {
  return FALSE;
}
inline BOOL VerQueryValue(LPCVOID pBlock, LPCSTR lpSubBlock, LPVOID *lplpBuffer,
                          PUINT puLen) {
  return FALSE;
}
inline BOOL GetFileTime(HANDLE hFile, LPFILETIME lpCreationTime,
                        LPFILETIME lpLastAccessTime,
                        LPFILETIME lpLastWriteTime) {
  return FALSE;
}
typedef void(__cdecl *_beginthread_proc_type)(void *);
inline uintptr_t _beginthread(_beginthread_proc_type _StartAddress,
                              unsigned _StackSize, void *_ArgList) {
  pthread_t thread;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  if (_StackSize > 0) {
    pthread_attr_setstacksize(&attr, _StackSize);
  }

  // Wrapper for pthread_create since return types differ
  struct Wrapper {
    _beginthread_proc_type start;
    void *arg;
    static void *run(void *p) {
      Wrapper *w = (Wrapper *)p;
      w->start(w->arg);
      delete w;
      return NULL;
    }
  };

  Wrapper *w = new Wrapper{_StartAddress, _ArgList};
  if (pthread_create(&thread, &attr, Wrapper::run, w) != 0) {
    delete w;
    pthread_attr_destroy(&attr);
    return (uintptr_t)-1;
  }
  pthread_attr_destroy(&attr);
  return (uintptr_t)thread;
}

inline BOOL WINAPI TerminateThread(HANDLE hThread, DWORD dwExitCode) {
#ifdef __APPLE__
  // pthread_cancel is available but often discouraged.
  // In many cases, we might want to just let it run or use a flag.
  // But for compatibility with TerminateThread, this is the closest.
  return pthread_cancel((pthread_t)hThread) == 0;
#else
  return TRUE;
#endif
}

#define THREAD_PRIORITY_LOWEST (-2)
#define THREAD_PRIORITY_BELOW_NORMAL (-1)
#define THREAD_PRIORITY_NORMAL (0)
#define THREAD_PRIORITY_ABOVE_NORMAL (1)
#define THREAD_PRIORITY_HIGHEST (2)
#define THREAD_PRIORITY_TIME_CRITICAL (15)

inline BOOL WINAPI SetThreadPriority(HANDLE hThread, int nPriority) {
  struct sched_param param;
  int policy;
  if (pthread_getschedparam((pthread_t)hThread, &policy, &param) != 0) {
    return FALSE;
  }

  int min = sched_get_priority_min(policy);
  int max = sched_get_priority_max(policy);
  int normal = (min + max) / 2;

  switch (nPriority) {
  case THREAD_PRIORITY_LOWEST:
    param.sched_priority = min;
    break;
  case THREAD_PRIORITY_BELOW_NORMAL:
    param.sched_priority = (min + normal) / 2;
    break;
  case THREAD_PRIORITY_NORMAL:
    param.sched_priority = normal;
    break;
  case THREAD_PRIORITY_ABOVE_NORMAL:
    param.sched_priority = (max + normal) / 2;
    break;
  case THREAD_PRIORITY_HIGHEST:
  case THREAD_PRIORITY_TIME_CRITICAL:
    param.sched_priority = max;
    break;
  default:
    param.sched_priority = normal;
    break;
  }

  return pthread_setschedparam((pthread_t)hThread, policy, &param) == 0;
}
#define REALTIME_PRIORITY_CLASS 0x00000100
#define HIGH_PRIORITY_CLASS 0x00000080
#define ABOVE_NORMAL_PRIORITY_CLASS 0x00008000
#define NORMAL_PRIORITY_CLASS 0x00000020
#define BELOW_NORMAL_PRIORITY_CLASS 0x00004000
#define IDLE_PRIORITY_CLASS 0x00000040

inline int GetThreadPriority(HANDLE hThread) { return THREAD_PRIORITY_NORMAL; }
inline BOOL SetPriorityClass(HANDLE hProcess, DWORD dwPriorityClass) {
  return TRUE;
}
inline DWORD GetPriorityClass(HANDLE hProcess) { return NORMAL_PRIORITY_CLASS; }

typedef DWORD(WINAPI *LPTHREAD_START_ROUTINE)(LPVOID lpThreadParameter);
inline HANDLE CreateThread(LPVOID lpThreadAttributes, size_t dwStackSize,
                           LPTHREAD_START_ROUTINE lpStartAddress,
                           LPVOID lpParameter, DWORD dwCreationFlags,
                           void *lpThreadId) {
  return NULL;
}

inline LONG RegOpenKeyEx(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions,
                         REGSAM samDesired, HKEY *phkResult) {
  return 1;
}
inline LONG RegQueryValueEx(HKEY hKey, LPCSTR lpValueName, void *lpReserved,
                            void *lpType, LPBYTE lpData, void *lpcbData) {
  return 1;
}
inline LONG RegCreateKeyEx(HKEY hKey, LPCSTR lpSubKey, DWORD Reserved,
                           LPSTR lpClass, DWORD dwOptions, REGSAM samDesired,
                           void *lpSecurityAttributes, HKEY *phkResult,
                           void *lpdwDisposition) {
  return 1;
}
inline LONG RegSetValueEx(HKEY hKey, LPCSTR lpValueName, DWORD Reserved,
                          DWORD dwType, const BYTE *lpData, DWORD cbData) {
  return 1;
}
inline LONG RegCloseKey(HKEY hKey) { return 0; }
inline BOOL CreateDirectory(LPCSTR lpPathName, void *lpSecurityAttributes) {
  return mkdir(lpPathName, 0755) == 0;
}
inline HANDLE CreateFile(LPCSTR lpFileName, DWORD dwDesiredAccess,
                         DWORD dwShareMode, void *lpSecurityAttributes,
                         DWORD dwCreationDisposition,
                         DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
  return NULL;
}
inline void GetLocalTime(SYSTEMTIME *lpSystemTime) {
  time_t t = time(NULL);
  struct tm *tm_info = localtime(&t);
  if (tm_info) {
    lpSystemTime->wYear = tm_info->tm_year + 1900;
    lpSystemTime->wMonth = tm_info->tm_mon + 1;
    lpSystemTime->wDay = tm_info->tm_mday;
    lpSystemTime->wHour = tm_info->tm_hour;
    lpSystemTime->wMinute = tm_info->tm_min;
    lpSystemTime->wSecond = tm_info->tm_sec;
    lpSystemTime->wMilliseconds = 0;
  }
}
typedef BOOL(CALLBACK *WNDENUMPROC)(HWND, LPARAM);
inline BOOL EnumThreadWindows(DWORD dwThreadId, WNDENUMPROC lpfn,
                              LPARAM lParam) {
  return TRUE;
}
inline LONG GetWindowLong(HWND hWnd, int nIndex) { return 0; }
inline int MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
  return IDOK;
}
inline int MessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption,
                       UINT uType) {
  return IDOK;
}
#define MessageBox MessageBoxA

inline BOOL EndPaint(HWND hWnd, const PAINTSTRUCT *lpPaint) { return FALSE; }
inline BOOL SetForegroundWindow(HWND hWnd) { return TRUE; }
inline HWND GetForegroundWindow() { return (HWND)NULL; }
inline BOOL IsIconic(HWND hWnd) { return FALSE; }

typedef struct tagMSG {
  HWND hwnd;
  UINT message;
  WPARAM wParam;
  LPARAM lParam;
  DWORD time;
  POINT pt;
} MSG, *PMSG, *LPMSG;

#define PM_NOREMOVE 0x0000
#define PM_REMOVE 0x0001
#define PM_NOYIELD 0x0002

inline BOOL PeekMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin,
                         UINT wMsgFilterMax, UINT wRemoveMsg) {
  (void)lpMsg;
  (void)hWnd;
  (void)wMsgFilterMin;
  (void)wMsgFilterMax;
  (void)wRemoveMsg;
  return FALSE;
}
#define PeekMessage PeekMessageA

inline BOOL GetMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin,
                        UINT wMsgFilterMax) {
  (void)lpMsg;
  (void)hWnd;
  (void)wMsgFilterMin;
  (void)wMsgFilterMax;
  return FALSE;
}
#define GetMessage GetMessageA

inline BOOL TranslateMessage(const MSG *lpMsg) {
  (void)lpMsg;
  return TRUE;
}
inline LRESULT DispatchMessageA(const MSG *lpMsg) {
  (void)lpMsg;
  return 0;
}
#define DispatchMessage DispatchMessageA

inline DWORD SetErrorMode(DWORD uMode) {
  (void)uMode;
  return 0;
}
#define SEM_FAILCRITICALERRORS 0x0001

typedef DWORD EXECUTION_STATE;
#define ES_CONTINUOUS 0x80000000
#define ES_DISPLAY_REQUIRED 0x00000002
#define ES_SYSTEM_REQUIRED 0x00000001

inline EXECUTION_STATE SetThreadExecutionState(EXECUTION_STATE esFlags) {
  (void)esFlags;
  return 0;
}

inline BOOL IsWindowVisible(HWND hWnd) {
  (void)hWnd;
  return TRUE;
}
inline BOOL ShowWindow(HWND hWnd, int nCmdShow) {
  (void)hWnd;
  (void)nCmdShow;
  return TRUE;
}
inline BOOL UpdateWindow(HWND hWnd) {
  (void)hWnd;
  return TRUE;
}
inline LRESULT DefWindowProcA(HWND hWnd, UINT Msg, WPARAM wParam,
                              LPARAM lParam) {
  return 0;
}
inline LRESULT DefWindowProcW(HWND hWnd, UINT Msg, WPARAM wParam,
                              LPARAM lParam) {
  return 0;
}
#ifdef UNICODE
#define DefWindowProc DefWindowProcW
#else
#define DefWindowProc DefWindowProcA
#endif

#define IMAGE_BITMAP 0
#define LR_LOADFROMFILE 0x0010
#define LR_SHARED 0x8000

inline HANDLE LoadImageA(HINSTANCE hInst, LPCSTR name, UINT type, int cx,
                         int cy, UINT fuLoad) {
  return NULL;
}
#define LoadImage LoadImageA

inline HWND FindWindowA(LPCSTR lpClassName, LPCSTR lpWindowName) {
  return NULL;
}
#define FindWindow FindWindowA

inline BOOL GetWindowRect(HWND hWnd, LPRECT lpRect) { return FALSE; }
inline void PostQuitMessage(int nExitCode) {}
inline HDC BeginPaint(HWND hWnd, LPPAINTSTRUCT lpPaint) { return (HDC)NULL; }
inline int SaveDC(HDC hdc) { return 0; }
inline BOOL RestoreDC(HDC hdc, int nSaveDC) { return FALSE; }
inline BOOL BitBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight,
                   HDC hdcSrc, int nXSrc, int nYSrc, DWORD dwRop) {
  return FALSE;
}

inline BOOL SetWindowTextA(HWND hWnd, LPCSTR lpString) { return TRUE; }
inline BOOL SetWindowTextW(HWND hWnd, LPCWSTR lpString) { return TRUE; }
#ifdef UNICODE
#define SetWindowText SetWindowTextW
#else
#define SetWindowText SetWindowTextA
#endif

#ifndef MulDiv
inline int MulDiv(int a, int b, int c) {
  if (c == 0)
    return -1;
  return (int)(((int64_t)a * (int64_t)b) / (int64_t)c);
}
#endif

inline BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
                     LPDWORD lpNumberOfBytesRead, LPVOID lpOverlapped) {
  return 0;
}
inline BOOL CloseHandle(HANDLE hObject) { return 1; }
inline DWORD GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename,
                                DWORD nSize) {
  return 0;
}
inline DWORD GetModuleFileNameW(HMODULE hModule, LPWSTR lpFilename,
                                DWORD nSize) {
  return 0;
}
#ifdef UNICODE
#define GetModuleFileName GetModuleFileNameW
#define GetSystemDirectory GetSystemDirectoryW
#else
#define GetModuleFileName GetModuleFileNameA
#define GetSystemDirectory GetSystemDirectoryA
#endif
inline UINT GetSystemDirectoryA(LPSTR lpBuffer, UINT uSize) {
  if (lpBuffer && uSize > 0) {
    lpBuffer[0] = '\0';
  }
  return 0;
}
inline UINT GetSystemDirectoryW(LPWSTR lpBuffer, UINT uSize) {
  if (lpBuffer && uSize > 0) {
    lpBuffer[0] = L'\0';
  }
  return 0;
}
inline LPVOID GlobalAlloc(UINT uFlags, size_t dwBytes) {
  return malloc(dwBytes);
}
#define GlobalAllocPtr(uFlags, dwBytes) GlobalAlloc(uFlags, dwBytes)
inline LPVOID GlobalFree(LPVOID hMem) {
  free(hMem);
  return NULL;
}

inline HLOCAL LocalAlloc(UINT uFlags, size_t uBytes) {
  void *ptr = malloc(uBytes);
  if (ptr && (uFlags & LMEM_ZEROINIT)) {
    memset(ptr, 0, uBytes);
  }
  return ptr;
}

inline HLOCAL LocalFree(HLOCAL hMem) {
  free(hMem);
  return NULL;
}

inline HMODULE LoadLibraryA(LPCSTR lpLibFileName) { return NULL; }
inline HMODULE LoadLibraryW(LPCWSTR lpLibFileName) { return NULL; }
inline BOOL FreeLibrary(HMODULE hLibModule) { return TRUE; }
inline FARPROC GetProcAddress(HMODULE hModule, LPCSTR lpProcName) {
  return NULL;
}

inline HANDLE GetCurrentProcess() { return (HANDLE)-1; }
inline BOOL AllocConsole() { return TRUE; }
inline BOOL FreeConsole() { return TRUE; }
inline BOOL SetConsoleMode(HANDLE hConsoleHandle, DWORD dwMode) { return TRUE; }
inline BOOL GetConsoleScreenBufferInfo(
    HANDLE hConsoleOutput,
    PCONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo) {
  return TRUE;
}
inline BOOL
SetConsoleCursorInfo(HANDLE hConsoleOutput,
                     const CONSOLE_CURSOR_INFO *lpConsoleCursorInfo) {
  return TRUE;
}
inline BOOL GetNumberOfConsoleInputEvents(HANDLE hConsoleInput,
                                          LPDWORD lpNumberOfEvents) {
  if (lpNumberOfEvents)
    *lpNumberOfEvents = 0;
  return TRUE;
}
inline BOOL ReadConsoleInputA(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer,
                              DWORD nLength, LPDWORD lpNumberOfEventsRead) {
  if (lpNumberOfEventsRead)
    *lpNumberOfEventsRead = 0;
  return TRUE;
}
inline BOOL WriteFile(HANDLE hFile, LPCVOID lpBuffer,
                      DWORD nNumberOfBytesToWrite,
                      LPDWORD lpNumberOfBytesWritten, LPVOID lpOverlapped) {
  if (lpNumberOfBytesWritten)
    *lpNumberOfBytesWritten = nNumberOfBytesToWrite;
  return TRUE;
}
inline BOOL CopyFile(LPCSTR lpExistingFileName, LPCSTR lpNewFileName,
                     BOOL bFailIfExists) {
  return TRUE;
}

inline DWORD GetCurrentDirectory(DWORD nBufferLength, char *lpBuffer) {
  if (getcwd(lpBuffer, nBufferLength)) {
    return (DWORD)strlen(lpBuffer);
  }
  return 0;
}

inline DWORD GetFileAttributes(const char *lpFileName) {
  struct stat st;
  if (stat(lpFileName, &st) == 0) {
    return FILE_ATTRIBUTE_NORMAL;
  }
  return INVALID_FILE_ATTRIBUTES;
}
#define DRIVE_UNKNOWN 0
#define DRIVE_NO_ROOT_DIR 1
#define DRIVE_REMOVABLE 2
#define DRIVE_FIXED 3
#define DRIVE_REMOTE 4
#define DRIVE_CDROM 5
#define DRIVE_RAMDISK 6

inline UINT GetDriveType(const char *lpRootPathName) {
  (void)lpRootPathName;
  return DRIVE_FIXED;
}
inline BOOL
GetVolumeInformation(const char *lpRootPathName, LPSTR lpVolumeNameBuffer,
                     DWORD nVolumeNameSize, LPDWORD lpVolumeSerialNumber,
                     LPDWORD lpMaximumComponentLength,
                     LPDWORD lpFileSystemFlags, LPSTR lpFileSystemNameBuffer,
                     DWORD nFileSystemNameSize) {
  (void)lpRootPathName;
  (void)lpVolumeNameBuffer;
  (void)nVolumeNameSize;
  (void)lpVolumeSerialNumber;
  (void)lpMaximumComponentLength;
  (void)lpFileSystemFlags;
  (void)lpFileSystemNameBuffer;
  (void)nFileSystemNameSize;
  return TRUE;
}

inline BOOL GetComputerName(LPSTR lpBuffer, LPDWORD nSize) { return TRUE; }
inline BOOL GetUserName(LPSTR lpBuffer, LPDWORD nSize) { return TRUE; }
inline BOOL SetNamedPipeHandleState(HANDLE hNamedPipe, LPDWORD lpMode,
                                    LPDWORD lpMaxCollectionCount,
                                    LPDWORD lpCollectDataTimeout) {
  return TRUE;
}
inline DWORD GetLastError() { return 0; }
inline HANDLE GetStdHandle(DWORD nStdHandle) { return NULL; }
inline BOOL SetConsoleCursorPosition(HANDLE hConsoleOutput,
                                     COORD dwCursorPosition) {
  return TRUE;
}
inline BOOL FillConsoleOutputAttribute(HANDLE hConsoleOutput, WORD wAttribute,
                                       DWORD nLength, COORD dwWriteCoord,
                                       LPDWORD lpNumberOfAttrsWritten) {
  return TRUE;
}
inline BOOL FillConsoleOutputCharacterA(HANDLE hConsoleOutput, CHAR cCharacter,
                                        DWORD nLength, COORD dwWriteCoord,
                                        LPDWORD lpNumberOfCharsWritten) {
  return TRUE;
}

inline HGLOBAL GlobalReAlloc(HGLOBAL hMem, size_t dwBytes, UINT uFlags) {
  return realloc(hMem, dwBytes);
}
#ifdef __APPLE__
#include <malloc/malloc.h>
#define malloc_size(p) malloc_size(p)
#else
#define malloc_size(p) 0
#endif

inline size_t GlobalSize(HGLOBAL hMem) {
  if (!hMem)
    return 0;
#ifdef __APPLE__
  return malloc_size(hMem);
#else
  return 0;
#endif
}

inline LPVOID GlobalLock(HGLOBAL hMem) { return hMem; }
inline BOOL GlobalUnlock(HGLOBAL hMem) { return TRUE; }
inline HGLOBAL GlobalHandle(LPCVOID p) { return (HGLOBAL)p; }

inline BOOL QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  lpPerformanceCount->QuadPart = (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;
  return TRUE;
}
inline BOOL QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency) {
  lpFrequency->QuadPart = 1000000;
  return TRUE;
}

typedef struct _SECURITY_ATTRIBUTES {
  DWORD nLength;
  LPVOID lpSecurityDescriptor;
  BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

typedef struct _STARTUPINFOW {
  DWORD cb;
  LPWSTR lpReserved;
  LPWSTR lpDesktop;
  LPWSTR lpTitle;
  DWORD dwX;
  DWORD dwY;
  DWORD dwXSize;
  DWORD dwYSize;
  DWORD dwXCountChars;
  DWORD dwYCountChars;
  DWORD dwFillAttribute;
  DWORD dwFlags;
  WORD wShowWindow;
  WORD cbReserved2;
  LPBYTE lpReserved2;
  HANDLE hStdInput;
  HANDLE hStdOutput;
  HANDLE hStdError;
} STARTUPINFOW, *LPSTARTUPINFOW;

typedef struct _PROCESS_INFORMATION {
  HANDLE hProcess;
  HANDLE hThread;
  DWORD dwProcessId;
  DWORD dwThreadId;
} PROCESS_INFORMATION, *PPROCESS_INFORMATION, *LPPROCESS_INFORMATION;

#define HANDLE_FLAG_INHERIT 0x00000001
#define STARTF_FORCEOFFFEEDBACK 0x00000080
#define STARTF_USESTDHANDLES 0x00000100
#define CREATE_NO_WINDOW 0x08000000
#define NORMAL_PRIORITY_CLASS 0x00000020

inline BOOL SetHandleInformation(HANDLE hObject, DWORD dwMask, DWORD dwFlags) {
  return TRUE;
}

inline HANDLE CreateJobObjectW(LPSECURITY_ATTRIBUTES lpJobAttributes,
                               LPCWSTR lpName) {
  return NULL;
}

#define JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE 0x00002000

typedef enum _JOBOBJECTINFOCLASS {
  JobObjectBasicLimitInformation = 2,
  JobObjectBasicUIRestrictions = 4,
  JobObjectSecurityLimitInformation = 5,
  JobObjectEndOfJobTimeInformation = 6,
  JobObjectAssociateCompletionPortInformation = 7,
  JobObjectBasicAndIoAccountingInformation = 8,
  JobObjectExtendedLimitInformation = 9,
} JOBOBJECTINFOCLASS;

typedef struct _JOBOBJECT_BASIC_LIMIT_INFORMATION {
  LARGE_INTEGER PerProcessUserTimeLimit;
  LARGE_INTEGER PerJobUserTimeLimit;
  DWORD LimitFlags;
  SIZE_T MinimumWorkingSetSize;
  SIZE_T MaximumWorkingSetSize;
  DWORD ActiveProcessLimit;
  ULONG_PTR Affinity;
  DWORD PriorityClass;
  DWORD SchedulingClass;
} JOBOBJECT_BASIC_LIMIT_INFORMATION, *PJOBOBJECT_BASIC_LIMIT_INFORMATION;

typedef struct _IO_COUNTERS {
  ULONGLONG ReadOperationCount;
  ULONGLONG WriteOperationCount;
  ULONGLONG OtherOperationCount;
  ULONGLONG ReadTransferCount;
  ULONGLONG WriteTransferCount;
  ULONGLONG OtherTransferCount;
} IO_COUNTERS;

typedef struct _JOBOBJECT_EXTENDED_LIMIT_INFORMATION {
  JOBOBJECT_BASIC_LIMIT_INFORMATION BasicLimitInformation;
  IO_COUNTERS IoInfo;
  SIZE_T ProcessMemoryLimit;
  SIZE_T JobMemoryLimit;
  SIZE_T PeakProcessMemoryLimit;
  SIZE_T PeakJobMemoryLimit;
} JOBOBJECT_EXTENDED_LIMIT_INFORMATION, *PJOBOBJECT_EXTENDED_LIMIT_INFORMATION;

inline BOOL SetInformationJobObject(
    HANDLE hJob, JOBOBJECTINFOCLASS JobObjectInformationClass,
    LPVOID lpJobObjectInformation, DWORD cbJobObjectInformationLength) {
  return TRUE;
}
inline BOOL AssignProcessToJobObject(HANDLE hJob, HANDLE hProcess) {
  return TRUE;
}
inline BOOL PeekNamedPipe(HANDLE hNamedPipe, LPVOID lpBuffer, DWORD nBufferSize,
                          LPDWORD lpBytesRead, LPDWORD lpTotalBytesAvail,
                          LPDWORD lpBytesLeftThisMessage) {
  return TRUE;
}
inline BOOL GetExitCodeProcess(HANDLE hProcess, LPDWORD lpExitCode) {
  return TRUE;
}

inline BOOL CreatePipe(PHANDLE hReadPipe, PHANDLE hWritePipe,
                       LPSECURITY_ATTRIBUTES /*lpPipeAttributes*/,
                       DWORD /*nSize*/) {
  if (hReadPipe) {
    *hReadPipe = nullptr;
  }
  if (hWritePipe) {
    *hWritePipe = nullptr;
  }
  return TRUE;
}

inline BOOL CreateProcessW(LPCWSTR /*lpApplicationName*/,
                           LPWSTR /*lpCommandLine*/,
                           LPSECURITY_ATTRIBUTES /*lpProcessAttributes*/,
                           LPSECURITY_ATTRIBUTES /*lpThreadAttributes*/,
                           BOOL /*bInheritHandles*/, DWORD /*dwCreationFlags*/,
                           LPVOID /*lpEnvironment*/,
                           LPCWSTR /*lpCurrentDirectory*/,
                           LPSTARTUPINFOW /*lpStartupInfo*/,
                           LPPROCESS_INFORMATION lpProcessInformation) {
  if (lpProcessInformation) {
    memset(lpProcessInformation, 0, sizeof(PROCESS_INFORMATION));
  }
  return TRUE;
}

// DInput stubs removed - use real dinput.h

#ifdef __cplusplus
}
#endif

#ifndef OutputDebugString
#ifndef _WIN32
#define OutputDebugString(str) printf("%s\n", str)
#endif
#endif

#ifndef Sleep
#define Sleep(ms) usleep((ms) * 1000)
#endif

#define closesocket close
#define _strnicmp strncasecmp
#define _stricmp strcasecmp
#define ERROR_SUCCESS 0L
#define ERROR_FILE_NOT_FOUND 2L
#define ERROR_INVALID_ACCESS 12L
#define ERROR_READ_FAULT 30L
#define FACILITY_WIN32 7
#define ERROR_FILE_EXISTS 80L
#define ERROR_OLD_WIN_VERSION 1150L
#define ERROR_NOT_READY 21L
#define ERROR_BUSY 170L
#define ERROR_ALREADY_INITIALIZED 1247L
#define ERROR_BAD_DRIVER_LEVEL 1131L
#define ERROR_RMODE_APP 1153L
#define E_ACCESSDENIED ((HRESULT)0x80070005L)

inline void _splitpath(const char *path, char *drive, char *dir, char *fname,
                       char *ext) {
  if (drive)
    *drive = '\0';
  if (dir)
    *dir = '\0';
  if (fname)
    *fname = '\0';
  if (ext)
    *ext = '\0';
}

#define _chmod chmod
#define _stat stat
#define _S_IWRITE S_IWUSR
#define _S_IREAD S_IRUSR

// AVI Support
typedef void *PAVIFILE;
typedef void *PAVISTREAM;

typedef struct _AVISTREAMINFOA {
  DWORD fccType;
  DWORD fccHandler;
  DWORD dwFlags;
  DWORD dwCaps;
  WORD wPriority;
  WORD wLanguage;
  DWORD dwScale;
  DWORD dwRate;
  DWORD dwStart;
  DWORD dwLength;
  DWORD dwInitialFrames;
  DWORD dwSuggestedBufferSize;
  DWORD dwQuality;
  DWORD dwSampleSize;
  RECT rcFrame;
  DWORD dwEditCount;
  DWORD dwFormatChangeCount;
  CHAR szName[64];
} AVISTREAMINFOA, *LPAVISTREAMINFOA;

typedef AVISTREAMINFOA AVISTREAMINFO;

#define mmioFOURCC(ch0, ch1, ch2, ch3)                                         \
  ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |                            \
   ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24))

#define streamtypeVIDEO mmioFOURCC('v', 'i', 'd', 's')
#define AVIIF_KEYFRAME 0x00000010L

inline HRESULT AVIFileCreateStream(PAVIFILE pfile, PAVISTREAM *ppavi,
                                   AVISTREAMINFO *psi) {
  return 0;
}
inline HRESULT AVIStreamSetFormat(PAVISTREAM pavi, LONG lPos, LPVOID lpFormat,
                                  LONG cbFormat) {
  return 0;
}
inline ULONG AVIStreamRelease(PAVISTREAM pavi) { return 0; }
inline ULONG AVIFileRelease(PAVIFILE pfile) { return 0; }
inline void AVIFileExit() {}
inline void AVIFileInit() {}
inline HRESULT AVIFileOpen(PAVIFILE *ppfile, LPCSTR szFile, UINT mode,
                           LPVOID pclsidHandler) {
  return 0;
}
inline HRESULT AVIStreamWrite(PAVISTREAM pavi, LONG lStart, LONG lSamples,
                              LPVOID lpBuffer, LONG cbBuffer, DWORD dwFlags,
                              LONG *plSampWritten, LONG *plBytesWritten) {
  return 0;
}

inline BOOL SetRect(LPRECT lprc, int xLeft, int yTop, int xRight, int yBottom) {
  if (!lprc) {
    return FALSE;
  }
  lprc->left = xLeft;
  lprc->top = yTop;
  lprc->right = xRight;
  lprc->bottom = yBottom;
  return TRUE;
}

inline int AddFontResource(LPCSTR lpszFilename) { return 1; }
inline BOOL RemoveFontResource(LPCSTR lpszFilename) { return TRUE; }

#define GlobalFreePtr(lp) GlobalFree(lp)

inline char *_itoa(int value, char *str, int base) {
  if (base == 10)
    sprintf(str, "%d", value);
  else if (base == 16)
    sprintf(str, "%x", value);
  else
    sprintf(str, "%d", value);
  return str;
}
#define itoa _itoa
#define wsprintf sprintf
#define wvsprintf vsprintf
#define _itoa(val, buf, radix)                                                 \
  sprintf(buf, (radix == 10 ? "%d" : (radix == 16 ? "%x" : "%o")), val)
#define _ultoa(val, buf, radix)                                                \
  sprintf(buf, (radix == 10 ? "%u" : (radix == 16 ? "%x" : "%o")), val)
#define _i64toa(val, buf, radix)                                               \
  sprintf(buf, (radix == 10 ? "%lld" : (radix == 16 ? "%llx" : "%llo")), val)
#define _ui64toa(val, buf, radix)                                              \
  sprintf(buf, (radix == 10 ? "%llu" : (radix == 16 ? "%llx" : "%llo")), val)

inline HRESULT CoInitialize(LPVOID pvReserved) { return S_OK; }
inline void CoUninitialize() {}

#endif

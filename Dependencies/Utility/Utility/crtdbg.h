#pragma once

#ifndef _CRTDBG_H_
#define _CRTDBG_H_

#ifndef _WIN32

#define _ASSERT(expr) ((void)0)
#define _ASSERTE(expr) ((void)0)
#define _RPT0(rptno, msg) ((void)0)
#define _RPT1(rptno, msg, arg1) ((void)0)
#define _RPT2(rptno, msg, arg1, arg2) ((void)0)
#define _RPT3(rptno, msg, arg1, arg2, arg3) ((void)0)
#define _RPT4(rptno, msg, arg1, arg2, arg3, arg4) ((void)0)
#define _RPTF0(rptno, msg) ((void)0)
#define _RPTF1(rptno, msg, arg1) ((void)0)
#define _RPTF2(rptno, msg, arg1, arg2) ((void)0)
#define _RPTF3(rptno, msg, arg1, arg2, arg3) ((void)0)
#define _RPTF4(rptno, msg, arg1, arg2, arg3, arg4) ((void)0)

#define _CRT_WARN 0
#define _CRT_ERROR 1
#define _CRT_ASSERT 2

#define _CrtSetReportMode(type, mode) (0)
#define _CrtSetReportFile(type, file) (0)
#define _CrtDbgBreak() ((void)0)

#define _CrtCheckMemory() (1)
#define _CrtDumpMemoryLeaks() (0)

#endif // _WIN32

#endif // _CRTDBG_H_

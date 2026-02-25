/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define _WIN32_WINNT 0x0400

#include "thread.h"
#include "Except.h"
#include "wwdebug.h"
#pragma warning ( push )
#pragma warning ( disable : 4201 )
#include "systimer.h"
#pragma warning ( pop )

#ifdef _WIN32
#include <process.h>
#include <windows.h>
#endif

#ifdef _UNIX
#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#endif

ThreadClass::ThreadClass(const char *thread_name, ExceptionHandlerType exception_handler) : handle(0), running(false), thread_priority(0)
{
	if (thread_name) {
		size_t nameLen = strlcpy(ThreadName, thread_name, ARRAY_SIZE(ThreadName));
		(void)nameLen; assert(nameLen < ARRAY_SIZE(ThreadName));
	} else {
		strcpy(ThreadName, "No name");
	}

	ExceptionHandler = exception_handler;
}

ThreadClass::~ThreadClass()
{
	Stop();
}

void __cdecl ThreadClass::Internal_Thread_Function(void* params)
{
	ThreadClass* tc=reinterpret_cast<ThreadClass*>(params);
	tc->running=true;
	tc->ThreadID = GetCurrentThreadId();

#ifdef _WIN32
	Register_Thread_ID(tc->ThreadID, tc->ThreadName);

#if defined(_MSC_VER)
	// MSVC supports structured exception handling (__try/__except)
	if (tc->ExceptionHandler != nullptr) {
		__try {
			tc->Thread_Function();
		} __except(tc->ExceptionHandler(GetExceptionCode(), GetExceptionInformation())) {};
	} else {
		tc->Thread_Function();
	}
#elif defined(__GNUC__) && defined(_WIN32)
	// GCC/MinGW-w64 doesn't support MSVC's __try/__except syntax
	// Call Thread_Function directly without SEH support
	tc->Thread_Function();
#else
	#error "ThreadClass::Internal_Thread_Function: Unsupported compiler. This code requires MSVC or GCC/MinGW-w64 targeting Windows."
#endif

#else //_WIN32
	tc->Thread_Function();
#endif //_WIN32

#ifdef _WIN32
	Unregister_Thread_ID(tc->ThreadID, tc->ThreadName);
#endif // _WIN32
	tc->handle=0;
	tc->ThreadID = 0;
}

void ThreadClass::Execute()
{
	WWASSERT(!handle);	// Only one thread at a time!
	#ifdef _UNIX
		// macOS: Thread not started. Background work is done on the main
		// thread (e.g. TextureLoader::Update drains _BackgroundQueue).
		// This avoids Metal thread-safety issues with D3D/Metal APIs.
		return;
	#else
		handle=_beginthread(&Internal_Thread_Function,0,this);
		SetThreadPriority((HANDLE)handle,THREAD_PRIORITY_NORMAL+thread_priority);
		WWDEBUG_SAY(("ThreadClass::Execute: Started thread %s, thread ID is %X", ThreadName, handle));
	#endif
}

void ThreadClass::Set_Priority(int priority)
{
	thread_priority=priority;
	#ifndef _UNIX
	if (handle) SetThreadPriority((HANDLE)handle,THREAD_PRIORITY_NORMAL+thread_priority);
	#endif
}

void ThreadClass::Stop(unsigned ms)
{
	running=false;
	#ifdef _UNIX
		// Wait for thread to finish (it checks 'running' flag)
		unsigned time=TIMEGETTIME();
		while (handle) {
			if ((TIMEGETTIME()-time)>ms) {
				// Timeout — force clear handle
				handle=0;
				break;
			}
			usleep(1000); // 1ms
		}
	#else
		unsigned time=TIMEGETTIME();
		while (handle) {
			if ((TIMEGETTIME()-time)>ms) {
				int res=TerminateThread((HANDLE)handle,0);
				res;	// just to silence compiler warnings
				WWASSERT(res);	// Thread still not killed!
				handle=0;
			}
			Sleep(0);
		}
	#endif
}

void ThreadClass::Sleep_Ms(unsigned ms)
{
	Sleep(ms);
}

#ifndef _UNIX
HANDLE test_event = ::CreateEvent (nullptr, FALSE, FALSE, "");
#endif

void ThreadClass::Switch_Thread()
{
	#ifdef _UNIX
		sched_yield();
		usleep(1000); // 1ms to prevent tight spin
	#else
		//	::SwitchToThread ();
		::WaitForSingleObject (test_event, 1);
		//	Sleep(1);	// Note! Parameter can not be 0 (or the thread switch doesn't occur)
	#endif
}

// Return calling thread's unique thread id
unsigned ThreadClass::_Get_Current_Thread_ID()
{
	#ifdef _UNIX
		return (unsigned)(uintptr_t)pthread_self();
	#else
		return GetCurrentThreadId();
	#endif
}

bool ThreadClass::Is_Running()
{
	return !!handle;
}

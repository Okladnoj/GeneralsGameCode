/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
*/

#include "thread.h"
#include "systimer.h"
#include "wwdebug.h"
#include <assert.h>
#include <string.h>

#ifdef _WIN32
#include <process.h>
#include <windows.h>
#else
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#endif

#ifndef _WIN32
static void Register_Thread_ID(unsigned, const char *) {}
static void Unregister_Thread_ID(unsigned, const char *) {}
#endif

ThreadClass::ThreadClass(const char *thread_name,
                         ExceptionHandlerType exception_handler)
    : handle(0), running(false), thread_priority(0) {
  if (thread_name) {
    strlcpy(ThreadName, thread_name, sizeof(ThreadName));
  } else {
    strcpy(ThreadName, "No name");
  }

  ExceptionHandler = exception_handler;
  ThreadID = 0;
}

ThreadClass::~ThreadClass() { Stop(); }

void __cdecl ThreadClass::Internal_Thread_Function(void *params) {
  ThreadClass *tc = reinterpret_cast<ThreadClass *>(params);
  tc->running = true;
  tc->ThreadID = GetCurrentThreadId();

#ifdef _WIN32
  Register_Thread_ID(tc->ThreadID, tc->ThreadName);
#endif

#ifdef __APPLE__
  pthread_setname_np(tc->ThreadName);
#endif

  tc->Thread_Function();

#ifdef _WIN32
  Unregister_Thread_ID(tc->ThreadID, tc->ThreadName);
#endif

  tc->handle = 0;
  tc->ThreadID = 0;
  tc->running = false;
}

void ThreadClass::Execute() {
  assert(!handle);

#ifdef _WIN32
  handle = _beginthread(&Internal_Thread_Function, 0, this);
  SetThreadPriority((HANDLE)handle, THREAD_PRIORITY_NORMAL + thread_priority);
#else
  pthread_t thread;
  if (pthread_create(&thread, nullptr,
                     (void *(*)(void *))Internal_Thread_Function, this) == 0) {
    handle = (unsigned long)thread;

    struct sched_param param;
    int policy;
    if (pthread_getschedparam(thread, &policy, &param) == 0) {
      param.sched_priority += thread_priority;
      pthread_setschedparam(thread, policy, &param);
    }
  }
#endif

  WWDEBUG_SAY(("ThreadClass::Execute: Started thread %s, thread ID is %lX",
               ThreadName, handle));
}

void ThreadClass::Set_Priority(int priority) {
  thread_priority = priority;
#ifdef _WIN32
  if (handle)
    SetThreadPriority((HANDLE)handle, THREAD_PRIORITY_NORMAL + thread_priority);
#else
  if (handle) {
    pthread_t thread = (pthread_t)handle;
    struct sched_param param;
    int policy;
    if (pthread_getschedparam(thread, &policy, &param) == 0) {
      param.sched_priority = priority;
      pthread_setschedparam(thread, policy, &param);
    }
  }
#endif
}

void ThreadClass::Stop(unsigned ms) {
  running = false;
  unsigned time = TIMEGETTIME();
  while (handle) {
    if ((TIMEGETTIME() - time) > ms) {
#ifdef _WIN32
      TerminateThread((HANDLE)handle, 0);
#else
      pthread_cancel((pthread_t)handle);
#endif
      handle = 0;
    }
    Sleep(0);
  }
}

void ThreadClass::Sleep_Ms(unsigned ms) { Sleep(ms); }

void ThreadClass::Switch_Thread() { Sleep(0); }

unsigned ThreadClass::_Get_Current_Thread_ID() { return GetCurrentThreadId(); }

bool ThreadClass::Is_Running() { return !!handle; }

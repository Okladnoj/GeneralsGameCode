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

#include "mutex.h"
#include "wwdebug.h"
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef _UNIX
#include <pthread.h>
#include <sys/time.h>
#include <cstdlib>
#endif

// ----------------------------------------------------------------------------

MutexClass::MutexClass(const char* name) : handle(nullptr), locked(false)
{
	#ifdef _UNIX
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_t* mtx = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(mtx, &attr);
		pthread_mutexattr_destroy(&attr);
		handle = mtx;
	#else
		handle=CreateMutex(nullptr,false,name);
		WWASSERT(handle);
	#endif
}

MutexClass::~MutexClass()
{
	#ifdef _UNIX
		if (handle) {
			pthread_mutex_destroy((pthread_mutex_t*)handle);
			free(handle);
		}
	#else
		WWASSERT(!locked); // Can't delete locked mutex!
		CloseHandle(handle);
	#endif
}

bool MutexClass::Lock(int time)
{
	#ifdef _UNIX
		if (!handle) return false;
		int res;
		if (time == WAIT_INFINITE) {
			res = pthread_mutex_lock((pthread_mutex_t*)handle);
		} else if (time == 0) {
			res = pthread_mutex_trylock((pthread_mutex_t*)handle);
		} else {
			struct timespec ts;
			struct timeval tv;
			gettimeofday(&tv, nullptr);
			ts.tv_sec = tv.tv_sec + time / 1000;
			ts.tv_nsec = tv.tv_usec * 1000 + (time % 1000) * 1000000;
			if (ts.tv_nsec >= 1000000000) {
				ts.tv_sec++;
				ts.tv_nsec -= 1000000000;
			}
			res = pthread_mutex_trylock((pthread_mutex_t*)handle);
			if (res != 0) {
				usleep(time * 1000);
				res = pthread_mutex_trylock((pthread_mutex_t*)handle);
			}
		}
		if (res != 0) return false;
		locked++;
		return true;
	#else
		int res = WaitForSingleObject(handle,time==WAIT_INFINITE ? INFINITE : time);
		if (res!=WAIT_OBJECT_0) return false;
		locked++;
		return true;
	#endif
}

void MutexClass::Unlock()
{
	#ifdef _UNIX
		if (handle && locked) {
			locked--;
			pthread_mutex_unlock((pthread_mutex_t*)handle);
		}
	#else
		WWASSERT(locked);
		locked--;
		int res=ReleaseMutex(handle);
		res;	// silence compiler warnings
		WWASSERT(res);
	#endif
}

// ----------------------------------------------------------------------------

MutexClass::LockClass::LockClass(MutexClass& mutex_,int time) : mutex(mutex_)
{
	failed=!mutex.Lock(time);
}

MutexClass::LockClass::~LockClass()
{
	if (!failed) mutex.Unlock();
}







// ----------------------------------------------------------------------------

CriticalSectionClass::CriticalSectionClass() : handle(nullptr), locked(false)
{
	#ifdef _UNIX
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_t* mtx = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(mtx, &attr);
		pthread_mutexattr_destroy(&attr);
		handle = mtx;
	#else
		handle=W3DNEWARRAY char[sizeof(CRITICAL_SECTION)];
		InitializeCriticalSection((CRITICAL_SECTION*)handle);
	#endif
}

CriticalSectionClass::~CriticalSectionClass()
{
	#ifdef _UNIX
		if (handle) {
			pthread_mutex_destroy((pthread_mutex_t*)handle);
			free(handle);
		}
	#else
		WWASSERT(!locked); // Can't delete locked mutex!
		DeleteCriticalSection((CRITICAL_SECTION*)handle);
		delete[] handle;
	#endif
}

void CriticalSectionClass::Lock()
{
	#ifdef _UNIX
		if (handle) {
			pthread_mutex_lock((pthread_mutex_t*)handle);
			locked++;
		}
	#else
		EnterCriticalSection((CRITICAL_SECTION*)handle);
		locked++;
	#endif
}

void CriticalSectionClass::Unlock()
{
	#ifdef _UNIX
		if (handle && locked) {
			locked--;
			pthread_mutex_unlock((pthread_mutex_t*)handle);
		}
	#else
		WWASSERT(locked);
		locked--;
		LeaveCriticalSection((CRITICAL_SECTION*)handle);
	#endif
}

// ----------------------------------------------------------------------------

CriticalSectionClass::LockClass::LockClass(CriticalSectionClass& critical_section) : CriticalSection(critical_section)
{
	CriticalSection.Lock();
}

CriticalSectionClass::LockClass::~LockClass()
{
	CriticalSection.Unlock();
}



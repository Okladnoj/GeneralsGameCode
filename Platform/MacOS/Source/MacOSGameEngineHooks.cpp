#include "MacOSGameEngineHooks.h"
#include "PreRTS.h"

#include "Common/Debug.h"
#include "Common/PerfTimer.h"
#include "Common/version.h"
#include <stdio.h>

#ifdef DUMP_PERF_STATS
static __int64 startTime64 = 0;
static __int64 freq64 = 0;
static char Buf[256];
#endif

void MacOS_LogBuildInfo() {
#ifdef DEBUG_LOGGING
  if (TheVersion) {
    DEBUG_LOG(("============================================================="
               "==================="));
    DEBUG_LOG(("Generals version %s", TheVersion->getAsciiVersion().str()));
    DEBUG_LOG(("Build date: %s", TheVersion->getAsciiBuildTime().str()));
    DEBUG_LOG(
        ("Build location: %s", TheVersion->getAsciiBuildLocation().str()));
    DEBUG_LOG(("Build user: %s", TheVersion->getAsciiBuildUser().str()));
    DEBUG_LOG(
        ("Build git revision: %s", TheVersion->getAsciiGitCommitCount().str()));
    DEBUG_LOG(
        ("Build git version: %s", TheVersion->getAsciiGitTagOrHash().str()));
    DEBUG_LOG(("Build git commit time: %s",
               TheVersion->getAsciiGitCommitTime().str()));
    DEBUG_LOG(
        ("Build git commit author: %s", Version::getGitCommitAuthorName()));
    DEBUG_LOG(("============================================================="
               "==================="));
  }
#endif
}

void MacOS_InitPerformanceTimers() {
#if defined(PERF_TIMERS) || defined(DUMP_PERF_STATS)
  DEBUG_LOG(("Calculating CPU frequency for performance timers."));
  InitPrecisionTimer();
#endif
#ifdef DUMP_PERF_STATS
  GetPrecisionTimerTicksPerSec(&freq64);
  GetPrecisionTimer(&startTime64);
#endif
}

void MacOS_LogSubsystemInitTime(const char *subsystemName) {
#ifdef DUMP_PERF_STATS
  __int64 endTime64;
  GetPrecisionTimer(&endTime64);
  sprintf(Buf,
          "------------------------------------------------------------After "
          "%s = %f seconds",
          subsystemName,
          ((double)(endTime64 - startTime64) / (double)(freq64)));
  startTime64 = endTime64;
  DEBUG_LOG(("%s", Buf));
#endif
}

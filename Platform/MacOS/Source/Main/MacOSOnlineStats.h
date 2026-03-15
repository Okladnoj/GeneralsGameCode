#pragma once

#ifdef __APPLE__
#include <string>

extern "C" std::string GenOnlineStats_FetchKVPairs(int profileId);
extern "C" void GenOnlineStats_UpdateKVPairs(int profileId,
                                              const std::string& kvPairs);
#endif

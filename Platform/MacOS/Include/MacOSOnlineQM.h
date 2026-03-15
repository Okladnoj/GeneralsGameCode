#pragma once

#ifdef __APPLE__
#include <vector>
#include <cstdint>

bool GenOnlineQM_Register(int playlistId, const std::vector<int>& mapIndices,
                          unsigned int exeCRC, unsigned int iniCRC);
bool GenOnlineQM_Widen();
bool GenOnlineQM_Deregister();

#endif

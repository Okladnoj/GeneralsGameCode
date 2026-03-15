#pragma once

#ifdef __APPLE__

enum class GenMapDownloadState : int {
    Idle = 0,
    Downloading = 1,
    Extracting = 2,
    Done = 3,
    Error = 4
};

bool GenMapDownloader_HasRankedMaps(void);

void GenMapDownloader_ShowWindow(void);

GenMapDownloadState GenMapDownloader_GetState(void);

int GenMapDownloader_GetProgress(void);

#endif // __APPLE__

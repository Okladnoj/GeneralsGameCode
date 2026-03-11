set(GS_OPENSSL FALSE)
set(GAMESPY_SERVER_NAME "server.cnc-online.net")

if(APPLE)
    list(APPEND GS_COMPILE_DEFS _MACOSX)
endif()

FetchContent_Declare(
    gamespy
    GIT_REPOSITORY https://github.com/Okladnoj/GamespySDK.git
    GIT_TAG        feature/macos-port
)

FetchContent_MakeAvailable(gamespy)

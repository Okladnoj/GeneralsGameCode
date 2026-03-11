set(GS_OPENSSL FALSE)
set(GAMESPY_SERVER_NAME "server.cnc-online.net")

if(APPLE)
    list(APPEND GS_COMPILE_DEFS _MACOSX)
endif()

add_compile_definitions(
    GAMESPY_PATCHING_HOSTNAME="${GAMESPY_SERVER_NAME}"
    GAMESPY_PATCHING_URL="http://${GAMESPY_SERVER_NAME}/servserv/"
)

FetchContent_Declare(
    gamespy
    GIT_REPOSITORY https://github.com/Okladnoj/GamespySDK.git
    GIT_TAG        feature/macos-port
)

FetchContent_MakeAvailable(gamespy)

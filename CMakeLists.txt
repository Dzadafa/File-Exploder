cmake_minimum_required(VERSION 3.20)
project(MyFileManager LANGUAGES C CXX ASM_NASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(UNICODE _UNICODE)

if(WIN32)
    set(CMAKE_ASM_NASM_OBJECT_FORMAT win64)
endif()

include_directories(src)

set(SOURCES
    src/main.cpp
    src/core.asm
    src/ui/MainWindow.cpp
    src/ui/FileList.cpp
    src/sys/FileSystem.cpp
    src/sys/History.cpp
)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

if(MINGW)
    target_link_options(${PROJECT_NAME} PRIVATE -municode)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE user32 kernel32 gdi32 shell32 comctl32 ole32 ntdll)

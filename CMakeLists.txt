# CMakeLists.txt for the static library
cmake_minimum_required (VERSION 3.2...4.2)

set(LIB_NAME interface)

# Enable CMake support for C languages
enable_language(C ASM)

add_library(${LIB_NAME} STATIC Interface.c )

add_subdirectory(./CircBuff circbuff)
add_subdirectory(./CRC crcinterface)

target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${LIB_NAME} PUBLIC wheap circbuff crcinterface)



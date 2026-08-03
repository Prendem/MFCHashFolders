cmake_minimum_required(VERSION 3.15)
project(MyMfcApp CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MFC_FLAG 2)

add_compile_definitions(_UNICODE UNICODE)

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h" "src/*.hpp")
file(GLOB_RECURSE RESOURCES "res/*")

add_executable(MfcHashFolders WIN32 ${SOURCES} ${HEADERS} ${RESOURCES})
target_include_directories(MfcHashFolders PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/res"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
source_group("Resource Files" FILES ${RESOURCES})
source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})
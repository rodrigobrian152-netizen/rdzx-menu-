cmake_minimum_required(VERSION 3.22.1)
project(zdrx CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(zdrx
        entry.cpp
        mem.cpp
        core.cpp
        net.cpp
        esp.cpp
        aim.cpp
        calibrar.cpp
        offsets.cpp)

target_compile_options(zdrx PRIVATE -O2 -fPIE -Wall -Wno-unused-variable)
target_link_options(zdrx PRIVATE -pie)
target_link_libraries(zdrx log)

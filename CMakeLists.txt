cmake_minimum_required (VERSION 2.6)
project (perf-map-agent)

find_package(JNI)
if (JNI_FOUND)
    message (STATUS "JNI_INCLUDE_DIRS=${JNI_INCLUDE_DIRS}")
    message (STATUS "JNI_LIBRARIES=${JNI_LIBRARIES}")
    message (STATUS "JAVA_INCLUDE_PATH=${JAVA_INCLUDE_PATH}")
    message (STATUS "JAVA_INCLUDE_PATH2=${JAVA_INCLUDE_PATH2}")
endif()
INCLUDE_DIRECTORIES(${JAVA_INCLUDE_PATH})
#INCLUDE_DIRECTORIES(${JAVA_INCLUDE_PATH2})

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_BUILD_TYPE Release)

# "GCC, the GNU C/C++ compiler, supports ‘-g’ with or without ‘-O’, making it
# possible to debug optimized code. We recommend that you always use ‘-g’
# whenever you compile a program."
# https://www.sourceware.org/gdb/current/onlinedocs/gdb.html
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

add_library(perfmap SHARED perf-map-agent.c)

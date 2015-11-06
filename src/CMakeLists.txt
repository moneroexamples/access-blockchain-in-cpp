cmake_minimum_required(VERSION 3.3)

project(myxrm)

set(SOURCE_HEADERS
        MicroCore.h
		tools.h
		monero_headers.h)

set(SOURCE_FILES
        MicroCore.cpp
		tools.cpp)

# make static library called libmyxrm
# that we are going to link to
# in the root CMakeLists.txt file
add_library(myxrm
		STATIC
		${SOURCE_FILES})

cmake_minimum_required(VERSION 3.3)

project(myxrm)

set(SOURCE_HEADERS
        MicroCore.h
		tools.h
		monero_headers.h)

set(SOURCE_FILES
        MicroCore.cpp
		tools.cpp)


add_library(myxrm
		SHARED
		${SOURCE_FILES}
		${SOURCE_HEADERS}
		)
#
#target_link_libraries(myxrm
#		LINK_PRIVATE
#	    	cryptonote_core
#			blockchain_db)
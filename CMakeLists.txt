cmake_minimum_required(VERSION 3.3)
project(xmreg01)

set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -std=c++11")



find_package(Boost COMPONENTS
        system
        filesystem
        thread
        date_time
        chrono
        regex
        serialization
        program_options
        REQUIRED)


include_directories(${Boost_INCLUDE_DIRS})


set(MONNERO_LIBS_DIR
        /opt/bitmonero-dev/libs)

set(MONERO_HEADERS_DIR
        /opt/bitmonero-dev/src)


include_directories(
        ${MONERO_HEADERS_DIR}/src
        ${MONERO_HEADERS_DIR}/external
        ${MONERO_HEADERS_DIR}/contrib/epee/include
        ${MONERO_HEADERS_DIR}/external/db_drivers/liblmdb64)




add_library(common STATIC IMPORTED)
set_property(TARGET common PROPERTY IMPORTED_LOCATION ${MONNERO_LIBS_DIR}/libcommon.a)


add_library(blocks STATIC IMPORTED)
set_property(TARGET blocks PROPERTY IMPORTED_LOCATION ${MONNERO_LIBS_DIR}/libblocks.a)


add_library(crypto STATIC IMPORTED)
set_property(TARGET crypto
        PROPERTY IMPORTED_LOCATION ${MONNERO_LIBS_DIR}/libcrypto.a)


add_library(cryptonote_core STATIC IMPORTED)
set_property(TARGET cryptonote_core
        PROPERTY IMPORTED_LOCATION ${MONNERO_LIBS_DIR}/libcryptonote_core.a)


add_library(blockchain_db STATIC IMPORTED)
set_property(TARGET blockchain_db
        PROPERTY IMPORTED_LOCATION ${MONNERO_LIBS_DIR}/libblockchain_db.a)

add_library(lmdb STATIC IMPORTED)
set_property(TARGET lmdb
        PROPERTY IMPORTED_LOCATION ${MONNERO_LIBS_DIR}/liblmdb.a)


add_subdirectory(src/)


set(SOURCE_FILES
        main.cpp)

add_executable(xmreg01
        ${SOURCE_FILES})


target_link_libraries(xmreg01
        myxrm
        cryptonote_core
        blockchain_db
        crypto
        blocks
        common
        lmdb
        ${Boost_LIBRARIES}
        pthread
        unbound
        )
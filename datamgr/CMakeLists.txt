cmake_minimum_required (VERSION 2.6)
project (DATAMGR)

INCLUDE_DIRECTORIES( ${MYSQL_INCLUDE_DIR} )

add_definitions(-std=c++11)
#add_definitions(-fpermissive)
add_definitions(-Wno-write-strings)
add_definitions(-Wno-unused-result)
add_definitions(-ggdb)
add_definitions(-fpermissive)
#add_definitions(-Wall)
add_definitions(-O3)

set(FILEPATH_DATAMGR ${CMAKE_CURRENT_SOURCE_DIR} )

set(SRC_DATAMGR
    ${FILEPATH_DATAMGR}/datamgr.cpp
    ${FILEPATH_DATAMGR}/main.cpp
  )



add_executable(datamgr ${SRC_DATAMGR} )
#add_executable(datamgr.conf ${SRC_DATAMGR} )
TARGET_LINK_LIBRARIES( datamgr ${CMAKE_SOURCE_DIR}/lib/libglobal.a ${MYSQL_CLIENT_LIBS} )


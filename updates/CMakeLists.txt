cmake_minimum_required (VERSION 2.6) 
project (UPDATESRV) 

INCLUDE_DIRECTORIES( ${MYSQL_INCLUDE_DIR} ) 

add_definitions(-std=c++11) 
#add_definitions(-fpermissive) 
add_definitions(-Wno-write-strings) 
add_definitions(-Wno-unused-result) 
#add_definitions(-ggdb) 
add_definitions(-fpermissive) 
#add_definitions(-Wall) 
add_definitions(-O3) 

set(FILEPATH_UPDATE ${CMAKE_CURRENT_SOURCE_DIR} ) 
set(SRC_UPDATE
 ${FILEPATH_UPDATE}/main.cpp 
 ) 
 add_executable(updatesrv ${SRC_UPDATE} ) 
# add_executable(update.conf ${SRC_UPDATE} ) 
 TARGET_LINK_LIBRARIES( updatesrv ${CMAKE_SOURCE_DIR}/lib/libglobal.a ${MYSQL_CLIENT_LIBS} )

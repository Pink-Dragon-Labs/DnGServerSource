cmake_minimum_required (VERSION 2.6) 
project (ALL_PROJECTS) 

include(FindMySQL.cmake) 

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin ) 
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib) 
INCLUDE_DIRECTORIES( "${PROJECT_BINARY_DIR}/global/" ) 
INCLUDE_DIRECTORIES( ${MYSQL_INCLUDE_DIR}/usr/include/mysql ) 

add_subdirectory(global) 
add_subdirectory(tokenize)
add_subdirectory(router) 
add_subdirectory(datamgr) 
add_subdirectory(updates)
add_subdirectory(roommgr)

#add_library(global STATIC ${SRC_GLOBAL})
#add_executable(roommgr.exe ${SRC_ROOMMGR} ${SRC_GLOBAL})
#TARGET_LINK_LIBRARIES( roommgr.exe ${MYSQL_CLIENT_LIBS} )

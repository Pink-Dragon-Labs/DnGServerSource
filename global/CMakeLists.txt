cmake_minimum_required (VERSION 2.6)
project (GLOBAL)



INCLUDE_DIRECTORIES( ${MYSQL_INCLUDE_DIR} )

add_definitions(-std=c++11)
add_definitions(-fpermissive)
add_definitions(-Wno-write-strings)
add_definitions(-Wno-unused-result)
add_definitions(-ggdb)
#add_definitions(-Wall)
add_definitions(-O3)

set(FILEPATH_GLOBAL ${CMAKE_CURRENT_SOURCE_DIR} )

set(SRC_GLOBAL
    ${FILEPATH_GLOBAL}/ipc.cpp
    ${FILEPATH_GLOBAL}/file.cpp
    ${FILEPATH_GLOBAL}/tools.cpp
    ${FILEPATH_GLOBAL}/fatal.cpp
    #${FILEPATH_GLOBAL}/window.cpp
    ${FILEPATH_GLOBAL}/ipcpollmgr.cpp
    ${FILEPATH_GLOBAL}/configmgr.cpp
    ${FILEPATH_GLOBAL}/logmgr.cpp
    ${FILEPATH_GLOBAL}/ipcclient.cpp
    ${FILEPATH_GLOBAL}/list.cpp
    ${FILEPATH_GLOBAL}/crash.cpp
    ${FILEPATH_GLOBAL}/ipcmsg.cpp
    ${FILEPATH_GLOBAL}/counter.cpp
    ${FILEPATH_GLOBAL}/cgi.cpp
    ${FILEPATH_GLOBAL}/html.cpp
    ${FILEPATH_GLOBAL}/player.cpp
    ${FILEPATH_GLOBAL}/array.cpp
    ${FILEPATH_GLOBAL}/ipcserver.cpp
    ${FILEPATH_GLOBAL}/packdata.cpp
    ${FILEPATH_GLOBAL}/hash.cpp
    ${FILEPATH_GLOBAL}/new.cpp
    ${FILEPATH_GLOBAL}/malloc.cpp
    ${FILEPATH_GLOBAL}/sparse.cpp
    ${FILEPATH_GLOBAL}/tree.cpp
    ${FILEPATH_GLOBAL}/string.cpp
    ${FILEPATH_GLOBAL}/memorypool.cpp
    ${FILEPATH_GLOBAL}/sql.cpp
    ${FILEPATH_GLOBAL}/script.cpp
    ${FILEPATH_GLOBAL}/msgs.cpp
    ${FILEPATH_GLOBAL}/ipcencryption.cpp
    ${FILEPATH_GLOBAL}/generator.cpp
    ${FILEPATH_GLOBAL}/validate.cpp
    ${FILEPATH_GLOBAL}/cencryptionstring.cpp
    ${FILEPATH_GLOBAL}/stringcache.cpp
  )


add_library(global STATIC ${SRC_GLOBAL})


#add_library(global STATIC ${SRC_GLOBAL})
#add_executable(roommgr.exe ${SRC_ROOMMGR} ${SRC_GLOBAL})
#TARGET_LINK_LIBRARIES( roommgr.exe ${MYSQL_CLIENT_LIBS} )








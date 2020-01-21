cmake_minimum_required (VERSION 2.6)
project (ROUTER)

INCLUDE_DIRECTORIES( ${MYSQL_INCLUDE_DIR} )

add_definitions(-std=c++11)
#add_definitions(-fpermissive)
add_definitions(-Wno-write-strings)
add_definitions(-Wno-unused-result)
add_definitions(-ggdb)
add_definitions(-fpermissive)
#add_definitions(-Wall)
add_definitions(-O3)

set(FILEPATH_ROUTER ${CMAKE_CURRENT_SOURCE_DIR} )

set(SRC_ROUTER
    ${FILEPATH_ROUTER}/main.cpp
  )



add_executable(router ${SRC_ROUTER} )


#add_executable(router.conf ${SRC_ROUTER} )
TARGET_LINK_LIBRARIES( router ${CMAKE_SOURCE_DIR}/lib/libglobal.a ${MYSQL_CLIENT_LIBS} )

add_custom_command(TARGET router.conf
                        POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:router.conf> ../bin/router.conf)


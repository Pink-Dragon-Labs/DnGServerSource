cmake_minimum_required (VERSION 2.6)
project (TOKENIZE)

INCLUDE_DIRECTORIES( ${MYSQL_INCLUDE_DIR} )

add_definitions(-std=c++11)
#add_definitions(-fpermissive)
add_definitions(-Wno-write-strings)
add_definitions(-Wno-unused-result)
add_definitions(-ggdb)
add_definitions(-fpermissive)
#add_definitions(-Wall)
add_definitions(-O3)

set(FILEPATH_TOKENIZER ${CMAKE_CURRENT_SOURCE_DIR} )

set(SRC_TOKENIZE
    ${FILEPATH_TOKENIZER}/main.cpp
  )



add_executable(tokenize ${SRC_TOKENIZE} )
TARGET_LINK_LIBRARIES( tokenize ${CMAKE_SOURCE_DIR}/lib/libglobal.a ${MYSQL_CLIENT_LIBS} )

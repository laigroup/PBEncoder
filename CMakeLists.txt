PROJECT(cmake_newKDBB)    # overwrites 'Makefile' (lower precedence than manually written 'GNUmakefile' wrt 'make')

CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

# set -std=c++14
SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# SET(CMAKE_CXX_FLAGS "-g -Ofast -static")
# close -static on mac
SET(CMAKE_CXX_FLAGS "-g -Ofast")

INCLUDE_DIRECTORIES(src/interface)

FILE(GLOB cpp_files "src/implement/*.cpp")

ADD_EXECUTABLE(Encoder ${cpp_files})

# TARGET_LINK_LIBRARIES(Test)

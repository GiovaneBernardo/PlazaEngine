cmake_minimum_required(VERSION 3.25)

project(PlazaLua)

add_library(PlazaLua STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/lapi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/lauxlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/lbaselib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/lcode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/lcorolib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/lctype.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/ldblib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/ldebug.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/ldo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/ldump.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/lfunc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/lgc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/linit.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/liolib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/llex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/lmathlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/lmem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/loadlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/lobject.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/lopcodes.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/loslib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/lparser.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/lstate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/lstring.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/lstrlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/ltable.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/ltablib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/ltm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/lua.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/lundump.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/lutf8lib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/lvm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/lzio.c
)

cmake_minimum_required(VERSION 3.2)
project(wwhd_rando_server CXX)
include("${DEVKITPRO}/wut/share/wut.cmake" REQUIRED)

add_executable(wwhd_rando_server main.cpp)
wut_create_rpx(wwhd_rando_server)
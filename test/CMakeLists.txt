cmake_minimum_required(VERSION 3.7)
project(wwhd_rando_tests CXX)

add_executable(yaz0test yaz0test.cpp "../filetypes/yaz0.cpp" "../utility/byteswap.cpp")
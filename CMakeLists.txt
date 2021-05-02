cmake_minimum_required(VERSION 3.7)
project(wwhd_rando_server CXX)

if(DEFINED DEVKITPRO)
  include("${DEVKITPRO}/wut/share/wut.cmake" REQUIRED)
  add_definitions(-DDEVKITPRO)
endif()

find_package(Threads REQUIRED)

if(CMAKE_USE_PTHREADS_INIT)
  message("here")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++11")
  set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()


add_executable(wwhd_rando_server main.cpp ProtocolServer.cpp json.hpp)
add_subdirectory("utility")
target_link_libraries(wwhd_rando_server Threads::Threads)
target_compile_features(wwhd_rando_server PUBLIC cxx_std_11)

if(DEFINED DEVKITPRO)
  wut_create_rpx(wwhd_rando_server)
endif()
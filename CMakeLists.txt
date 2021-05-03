cmake_minimum_required(VERSION 3.7)
project(wwhd_rando_server CXX)

if(DEFINED DEVKITPRO)
  include("${DEVKITPRO}/wut/share/wut.cmake" REQUIRED)
  add_definitions(-DDEVKITPRO)
else()
  find_package(Threads REQUIRED)
endif()

if(CMAKE_USE_PTHREADS_INIT)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++11")
  set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()


add_executable(wwhd_rando_server main.cpp ProtocolServer.cpp json.hpp)
add_subdirectory("utility")
set_property(TARGET wwhd_rando_server PROPERTY CXX_STANDARD 11)
if(CMAKE_USE_PTHREADS_INIT)
  target_link_libraries(wwhd_rando_server Threads::Threads)
endif()

if(DEFINED DEVKITPRO)
  wut_create_rpx(wwhd_rando_server)
endif()
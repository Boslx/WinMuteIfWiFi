cmake_minimum_required(VERSION 3.16)
project(WinMuteIfWiF)

set(CMAKE_CXX_STANDARD 14)

add_executable(WinMuteIfWiF src/WinMuteIfWifi.cpp src/WinMuteIfWifi.h src/WifiHelper.cpp src/WifiHelper.h)
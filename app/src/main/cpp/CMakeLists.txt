cmake_minimum_required(VERSION 3.22.1)
project(aaudio_player)

add_library(aaudio_player SHARED
        native-lib.cpp)

target_link_libraries(aaudio_player
        aaudio
        log)
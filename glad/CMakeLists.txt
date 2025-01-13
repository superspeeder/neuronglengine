cmake_minimum_required(VERSION 3.27)
project(glad)

add_library(glad src/gl.c)
target_include_directories(glad PUBLIC include/)

add_library(glad::glad ALIAS glad)
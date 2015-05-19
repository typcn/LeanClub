cmake_minimum_required(VERSION 2.8)
project (hiredis)

file(GLOB SRC_FILES "*.c" "*.h" "adapters/*.h")

add_library(hiredis ${SRC_FILES})
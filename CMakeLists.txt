cmake_minimum_required(VERSION 3.19.2)
project(tomato-db)

include(cmake_conf/common_conf.cmake)

add_subdirectory(common)
add_subdirectory(net)
add_subdirectory(db)
add_subdirectory(server)
add_subdirectory(exe)
add_subdirectory(third_party/googletest)
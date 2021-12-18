project(common)

include(../cmake_conf/gtest_conf.cmake)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(HEADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(${PROJECT_NAME} SHARED)
target_sources(${PROJECT_NAME}
    PRIVATE
        ${SRC_DIR}/tomato_allocator.cc
)

target_include_directories(${PROJECT_NAME}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

tomato_db_test("test/tomato_allocator_test.cc")
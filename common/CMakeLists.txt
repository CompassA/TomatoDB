project(common)

include(../cmake_conf/gtest_conf.cmake)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(HEADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(${PROJECT_NAME} SHARED)
target_sources(${PROJECT_NAME}
    PRIVATE
        ${SRC_DIR}/tomato_allocator.cc
        ${SRC_DIR}/tomato_codec.cc
        ${SRC_DIR}/tomato_crc32.cc
        ${HEADER_DIR}/tomato_skip_list.h
)

target_include_directories(${PROJECT_NAME}
    INTERFACE
        ${HEADER_DIR}
)

tomato_db_test("test/tomato_allocator_test.cc")
tomato_db_test("test/tomato_skip_list_test.cc")
tomato_db_test("test/tomato_codec_test.cc")
tomato_db_test("test/tomato_crc32_test.cc")
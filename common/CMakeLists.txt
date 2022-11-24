project(common)

include(../cmake_conf/gtest_conf.cmake)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(HEADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(${PROJECT_NAME} SHARED)
target_sources(${PROJECT_NAME}
    PRIVATE
        ${SRC_DIR}/allocator.cc
        ${SRC_DIR}/codec.cc
        ${SRC_DIR}/crc32.cc
        ${SRC_DIR}/posix_io.cc
        ${HEADER_DIR}/tomato_common/skip_list.h
)
add_library(tomato::common ALIAS common)

target_include_directories(${PROJECT_NAME}
    PUBLIC 
        ${HEADER_DIR}
)

tomato_db_test("test/tomato_allocator_test.cc")
tomato_db_test("test/tomato_skip_list_test.cc")
tomato_db_test("test/tomato_codec_test.cc")
tomato_db_test("test/tomato_crc32_test.cc")
tomato_db_test("test/tomato_posix_io_test.cc")
project(db)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(HEADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
include(../cmake_conf/gtest_conf.cmake)

add_library(${PROJECT_NAME} SHARED)
target_sources(${PROJECT_NAME}
    PRIVATE
        ${SRC_DIR}/tomato_memory_table.cc
        ${SRC_DIR}/tomato_sstable_builder.cc
)

target_include_directories(${PROJECT_NAME}
    INTERFACE
        ${HEADER_DIR}
)

target_link_libraries(${PROJECT_NAME} 
    PUBLIC
        common
)

tomato_db_test("test/tomato_memory_table_test.cc") 
tomato_db_test("test/tomato_sstable_builder_test.cc")  
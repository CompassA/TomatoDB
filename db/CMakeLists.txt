project(db)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(HEADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
include(../cmake_conf/gtest_conf.cmake)

add_library(${PROJECT_NAME} SHARED)
target_sources(${PROJECT_NAME}
    PRIVATE
        ${SRC_DIR}/memory_table.cc
        ${SRC_DIR}/sstable_builder.cc
)
add_library(tomato::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${HEADER_DIR}
)

target_link_libraries(${PROJECT_NAME} 
    PUBLIC
        tomato::common
)

tomato_db_test("test/tomato_memory_table_test.cc") 
tomato_db_test("test/tomato_sstable_builder_test.cc")  
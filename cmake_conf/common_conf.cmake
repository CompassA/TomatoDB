set(CMAKE_CXX_FLAGS 
        "${CAMKE_CXX_FLAGS}   \
        -std=c++11            \
        -g                    \
        -Wall                 \
        -Wextra               \
        -Wconversion          \
        -Wno-unused-parameter \
        -Wold-style-cast      \
        -Woverloaded-virtual  \
        -Wpointer-arith       \
        -Wshadow              \
        -Wwrite-strings       \
        -march=native         \
        -pthread")
add_link_options(-rdynamic)
add_library(core STATIC
    bitboard.cpp
    board.cpp
    movegen.cpp
    search.cpp
)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

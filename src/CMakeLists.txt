add_subdirectory(core)
add_subdirectory(engine)
add_subdirectory(training)

# We'll build the main executable later
add_executable(AlphaZeroChess src/main_training.cpp)
target_link_libraries(AlphaZeroChess core engine training)

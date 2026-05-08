add_subdirectory(core)
add_subdirectory(engine)
add_subdirectory(mcts)
add_subdirectory(training)

# We'll build the main executable later
add_executable(AlphaZeroChess main_training.cpp)
target_link_libraries(AlphaZeroChess core engine mcts training)

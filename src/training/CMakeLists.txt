add_library(training STATIC
    self_play.cpp
    data_storage.cpp
)

target_include_directories(training PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(training PUBLIC core engine)

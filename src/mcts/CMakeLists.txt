include(FetchContent)

# Download ONNX Runtime binary for Windows
FetchContent_Declare(
    onnxruntime
    URL "https://github.com/microsoft/onnxruntime/releases/download/v1.17.1/onnxruntime-win-x64-1.17.1.zip"
)
FetchContent_MakeAvailable(onnxruntime)

add_library(mcts STATIC
    onnx_evaluator.cpp
    mcts.cpp
)

target_include_directories(mcts PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${onnxruntime_SOURCE_DIR}/include
)

target_link_libraries(mcts PUBLIC core ${onnxruntime_SOURCE_DIR}/lib/onnxruntime.lib)

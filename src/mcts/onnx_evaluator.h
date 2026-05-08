#pragma once
#include "core/board.h"
#include <vector>
#include <string>

// Include ONNX Runtime C++ API
#include <onnxruntime_cxx_api.h>

namespace MCTS {

struct NetworkOutput {
    std::vector<float> policy; // 4672 probabilities
    float value;               // [-1, 1] evaluation
};

class NeuralNetworkEvaluator {
public:
    NeuralNetworkEvaluator(const std::string& model_path);
    ~NeuralNetworkEvaluator() = default;

    // Convert board state to 119-plane tensor and evaluate
    NetworkOutput evaluate(const BoardState& board);

private:
    Ort::Env env;
    Ort::Session session;
    
    // ONNX Runtime input/output names
    const char* input_names[1] = {"board_state"};
    const char* output_names[2] = {"policy", "value"};
};

} // namespace MCTS

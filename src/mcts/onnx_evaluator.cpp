#include "onnx_evaluator.h"
#include <iostream>

namespace MCTS {

NeuralNetworkEvaluator::NeuralNetworkEvaluator(const std::string& model_path)
    : env(ORT_LOGGING_LEVEL_WARNING, "AlphaZeroChess"), session(nullptr) {
#ifdef _WIN32
    std::wstring w_model_path(model_path.begin(), model_path.end());
    session = Ort::Session(env, w_model_path.c_str(), Ort::SessionOptions{nullptr});
#else
    session = Ort::Session(env, model_path.c_str(), Ort::SessionOptions{nullptr});
#endif
    std::cout << "Loaded ONNX model from " << model_path << std::endl;
}

NetworkOutput NeuralNetworkEvaluator::evaluate(const BoardState& board) {
    // Generate 119-plane tensor
    std::vector<int64_t> input_shape = {1, 119, 8, 8};
    std::vector<float> input_tensor(119 * 8 * 8, 0.0f);
    
    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    auto input = Ort::Value::CreateTensor<float>(
        memory_info, input_tensor.data(), input_tensor.size(),
        input_shape.data(), input_shape.size()
    );
    
    auto outputs = session.Run(
        Ort::RunOptions{nullptr},
        input_names, &input, 1,
        output_names, 2
    );
    
    NetworkOutput result;
    
    // Policy output: [1, 4672]
    auto policy_tensor = outputs[0].GetTensorMutableData<float>();
    result.policy.assign(policy_tensor, policy_tensor + 4672);
    
    // Value output: [1, 1]
    auto value_tensor = outputs[1].GetTensorMutableData<float>();
    result.value = value_tensor[0];
    
    return result;
}

} // namespace MCTS

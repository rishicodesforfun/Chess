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
    
    // Fill history planes (12 planes * 8 positions = 96 planes)
    // Also track repetitions (2 planes * 8 positions = 16 planes)
    const StateInfo* curr_st = board.get_state_info();

    for (int h = 0; h < 8; ++h) {
        if (!curr_st) break;

        // Fill piece planes for this historical position from saved bitboards
        for (int c = 0; c < COLOR_NB; ++c) {
            for (int pt = 0; pt < PIECE_TYPE_NB; ++pt) {
                Bitboard bb = curr_st->piece_bb[c][pt];
                while (bb) {
                    Square s = Bitboards::lsb(Bitboards::pop_lsb(bb));
                    int plane = h * 12 + (c == WHITE ? 0 : 6) + pt;
                    input_tensor[plane * 64 + rank_of(s) * 8 + file_of(s)] = 1.0f;
                }
            }
        }

        // Repetitions (planes 96-111)
        int rep_count = 0;
        const StateInfo* check_st = curr_st->previous;
        while (check_st) {
            if (check_st->zobrist_key == curr_st->zobrist_key) rep_count++;
            check_st = check_st->previous;
        }
        if (rep_count >= 1) {
            for (int i = 0; i < 64; ++i) input_tensor[(96 + h * 2) * 64 + i] = 1.0f;
        }
        if (rep_count >= 2) {
            for (int i = 0; i < 64; ++i) input_tensor[(96 + h * 2 + 1) * 64 + i] = 1.0f;
        }

        curr_st = curr_st->previous;
    }
    
    // Constant planes (112-118)
    // 112: Side to move (1 if white)
    if (board.side_to_move() == WHITE) {
        for (int i = 0; i < 64; ++i) input_tensor[112 * 64 + i] = 1.0f;
    }
    
    // 113: Total move count (normalized, e.g. fullmove / 100)
    float move_count = (float)board.fullmove_number() / 100.0f;
    for (int i = 0; i < 64; ++i) input_tensor[113 * 64 + i] = move_count;

    // 114-117: Castling rights
    int cr = board.castling_rights();
    if (cr & WHITE_OO)  for (int i = 0; i < 64; ++i) input_tensor[114 * 64 + i] = 1.0f;
    if (cr & WHITE_OOO) for (int i = 0; i < 64; ++i) input_tensor[115 * 64 + i] = 1.0f;
    if (cr & BLACK_OO)  for (int i = 0; i < 64; ++i) input_tensor[116 * 64 + i] = 1.0f;
    if (cr & BLACK_OOO) for (int i = 0; i < 64; ++i) input_tensor[117 * 64 + i] = 1.0f;

    // 118: No-progress count (halfmove clock / 100)
    float halfmove = (float)board.halfmove_clock() / 100.0f;
    for (int i = 0; i < 64; ++i) input_tensor[118 * 64 + i] = halfmove;

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

#pragma once
#include "core/board.h"
#include "core/movegen.h"
#include "onnx_evaluator.h"
#include <vector>

namespace MCTS {

struct MCTSNode {
    BoardState state;
    MCTSNode* parent;
    std::vector<MCTSNode*> children;
    std::vector<Move> moves;
    
    std::vector<float> visit_counts;
    std::vector<float> action_values;
    float prior_policy;
    float value_estimate;
    
    bool is_terminal;
    int visits;
    
    MCTSNode(const BoardState& s) 
        : state(s), parent(nullptr), visits(0), is_terminal(false), prior_policy(1.0f), value_estimate(0.0f) {}
    
    ~MCTSNode() {
        for (auto child : children) {
            delete child;
        }
    }
};

class MCTSEngine {
public:
    MCTSEngine(NeuralNetworkEvaluator& eval);
    
    Move search(const BoardState& root_state, int num_simulations);

private:
    NeuralNetworkEvaluator& nn_evaluator;
    const float c_puct = 4.0f;
    
    MCTSNode* select_leaf(MCTSNode* node);
    void expand_node(MCTSNode* node);
    float evaluate_leaf(MCTSNode* node);
    void backpropagate(MCTSNode* node, float value);
    Move select_best_move(MCTSNode* root);
};

} // namespace MCTS

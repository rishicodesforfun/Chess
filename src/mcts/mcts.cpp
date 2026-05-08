#include "mcts.h"
#include <cmath>

namespace MCTS {

MCTSEngine::MCTSEngine(NeuralNetworkEvaluator& eval) : nn_evaluator(eval) {}

Move MCTSEngine::search(const BoardState& root_state, int num_simulations) {
    MCTSNode* root = new MCTSNode(root_state);
    expand_node(root);
    
    for (int i = 0; i < num_simulations; i++) {
        MCTSNode* leaf = select_leaf(root);
        float value = evaluate_leaf(leaf);
        backpropagate(leaf, value);
    }
    
    Move best_move = select_best_move(root);
    delete root;
    return best_move;
}

MCTSNode* MCTSEngine::select_leaf(MCTSNode* node) {
    while (!node->is_terminal && !node->children.empty()) {
        int best_idx = -1;
        float best_puct = -1000000.0f;
        
        for (size_t i = 0; i < node->children.size(); i++) {
            float q = node->action_values[i] / (1.0f + node->visit_counts[i]);
            float u = c_puct * node->prior_policy * std::sqrt((float)node->visits) / (1.0f + node->visit_counts[i]);
            float puct = q + u;
            
            if (puct > best_puct) {
                best_puct = puct;
                best_idx = i;
            }
        }
        if (best_idx != -1) node = node->children[best_idx];
        else break;
    }
    return node;
}

void MCTSEngine::expand_node(MCTSNode* node) {
    node->moves = MoveGen::generate_legal_moves(node->state);
    if (node->moves.empty()) {
        node->is_terminal = true;
        return;
    }
    
    NetworkOutput nn_output = nn_evaluator.evaluate(node->state);
    for (size_t i = 0; i < node->moves.size(); i++) {
        MCTSNode* child = new MCTSNode(node->state);
        StateInfo st;
        child->state.do_move(node->moves[i], st);
        child->parent = node;
        
        node->children.push_back(child);
        node->visit_counts.push_back(0);
        node->action_values.push_back(0);
        child->prior_policy = nn_output.policy[i % 4672]; // Simplified mapping
    }
    node->value_estimate = nn_output.value;
}

float MCTSEngine::evaluate_leaf(MCTSNode* node) {
    if (node->is_terminal) {
        if (MoveGen::is_in_check(node->state)) return -1.0f;
        return 0.0f;
    }
    expand_node(node);
    return node->value_estimate;
}

void MCTSEngine::backpropagate(MCTSNode* node, float value) {
    while (node != nullptr) {
        node->visits++;
        if (node->parent != nullptr) {
            for (size_t i = 0; i < node->parent->children.size(); i++) {
                if (node->parent->children[i] == node) {
                    node->parent->visit_counts[i]++;
                    node->parent->action_values[i] += value;
                    break;
                }
            }
        }
        node = node->parent;
        value = -value; // Flip value for opposing side
    }
}

Move MCTSEngine::select_best_move(MCTSNode* root) {
    if (root->moves.empty()) return Move();
    int best_idx = 0;
    float best_visits = -1;
    for (size_t i = 0; i < root->visit_counts.size(); i++) {
        if (root->visit_counts[i] > best_visits) {
            best_visits = root->visit_counts[i];
            best_idx = i;
        }
    }
    return root->moves[best_idx];
}

} // namespace MCTS

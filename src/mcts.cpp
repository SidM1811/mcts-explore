#include <vector>
#include <cmath>
#include <cstdlib>
#include <iostream>

// Include the Game class header
#include "game_dynamics/tictactoe.hpp"

#define NUM_ROLLOUTS 100


template <typename Game>
class MCTSNode {
    constexpr static double INF = 1.0;
    public:
    using ActionT = typename Game::ActionT;
    using ActionIdxT = int;
    using RewardT = typename Game::RewardT;
    using PlayerType = typename Game::PlayerType;

    MCTSNode* parent;
    std::vector<MCTSNode*> children;
    int n_visits;
    double Q;
    bool is_expanded;
    int num_actions, num_explored_actions;
    PlayerType player;
    

    RewardT random_rollouts(Game& game_state, int num_rollouts){
        RewardT total_reward = {0, 0};
        for (int i = 0; i < num_rollouts; i++){
            Game rollout = game_state.copy();
            while (!rollout.is_terminal()){
                int action_idx = rand() % rollout.num_actions;
                rollout.step(action_idx);
            }
            total_reward += rollout.get_reward();
            // std::cout << "Reward: " << total_reward[0] << " " << total_reward[1] << std::endl;
        }
        return total_reward / num_rollouts;
    }

    MCTSNode(MCTSNode* parent){
        this->parent = parent;
        this->n_visits = 0;
        this->Q = 0;
        this->is_expanded = false;
    }

    void make_root(){
        parent = nullptr;
    }

    RewardT expand(Game& game_state){
        is_expanded = !game_state.is_terminal();
        num_actions = game_state.num_actions;
        num_explored_actions = 0;
        player = game_state.get_prev_player();
        children.resize(num_actions, nullptr);
        RewardT reward = random_rollouts(game_state, NUM_ROLLOUTS);
        Q = reward[player];
        return reward;
    }

    std::pair<MCTSNode*, ActionIdxT> most_visited(){
        ActionIdxT best_action = -1;
        for(ActionIdxT i = 0; i < num_actions; i++){
            MCTSNode* child = children[i];
            if (child == nullptr) continue;
            if (best_action == -1 || child->n_visits > children[best_action]->n_visits){
                best_action = i;
            }
        }
        return std::make_pair(children[best_action], best_action);
    }

    std::pair<MCTSNode*, ActionIdxT> select_child(){
        if (num_explored_actions < num_actions){
            // Select unexplored child - should be uniform but here deterministic
            for (ActionIdxT i = 0; i < num_actions; i++){
                MCTSNode* child = children[i];
                if (child == nullptr){
                    children[i] = new MCTSNode(this);
                    child = children[i];
                }
                assert(child != nullptr);
                if (child->n_visits == 0){
                    num_explored_actions++;
                    return std::make_pair(child, i);
                }
            }
        }

        // Select child with highest UCB - simple scan
        double best_uct = -INF;
        MCTSNode* best_child = nullptr;
        ActionIdxT best_action = -1;
        // Iterate over pairs of children and actions
        // The actions of the game state match with the MCTSNode children
        for (ActionIdxT i = 0; i < num_actions; i++){
            MCTSNode* child = children[i];
            double uct = child->Q / child->n_visits + sqrt(2 * log(n_visits) / child->n_visits);
            if (uct > best_uct){
                best_uct = uct;
                best_child = child;
                best_action = i;
            }
        }
        return std::make_pair(best_child, best_action);
    }
    void update_recursive(RewardT result){
        n_visits++;
        Q += (result[player] - Q) / n_visits;
        if (parent != nullptr){
            parent->update_recursive(result);
        }
    }

    void print(){
        std::cout << "Q: " << Q << std::endl;
        std::cout << "n_visits: " << n_visits << std::endl;
        std::cout << "num_actions: " << num_actions << std::endl;
        std::cout << "num_explored_actions: " << num_explored_actions << std::endl;
        std::cout << "is_expanded: " << is_expanded << std::endl;

        // for (int i = 0; i < num_actions; i++){
        //     if (children[i] != nullptr){
        //         std::cout << "Child " << i << std::endl;
        //         children[i]->print();
        //     }
        // }
    }
};

void run_sim(){
    TicTacToe game;
    MCTSNode<TicTacToe>* root = new MCTSNode<TicTacToe>(nullptr);
    int num_iters = 1000;

    for (int num_ply = 0; num_ply < 9; num_ply++){
        std::cout << "Ply: " << num_ply << std::endl;
        for (int i = 0; i < num_iters; i++){
            MCTSNode<TicTacToe>* node = root;
            TicTacToe state = game.copy();
            while (node->is_expanded){
                auto [child, action] = node->select_child();
                assert(child != nullptr);
                state.step(action);
                node = child;
            }
            auto reward = node->expand(state);
            node->update_recursive(reward);
        }
        auto [best_child, best_action] = root->most_visited();
        game.step(best_action);
        best_child->make_root();
        std::cout << "Best action: " << best_action << std::endl;
        root->print();
        game.print();
        root = best_child;
    }
}

void play_game(){
    
}

int main(){
    run_sim();
}
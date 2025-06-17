#include <vector>
#include <cmath>
#include <cstdlib>
#include <iostream>

// Include the Game class header
#include "game_dynamics/tictactoe.hpp"
#include "game_dynamics/connect4.hpp"
#include "batch_malloc.h"

#define NUM_ROLLOUTS 100

template <typename Game>
class MCTSNode {
    constexpr static double INF = 1e6;
    public:
    using ActionT = typename Game::ActionT;
    using ActionIdxT = int;
    using RewardT = typename Game::RewardT;
    using PlayerType = typename Game::PlayerType;

    MCTSNode<Game>* parent;
    std::vector<MCTSNode<Game>*> children;
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
                ActionIdxT action_idx = random_policy(rollout);
                rollout.step(action_idx);
            }
            total_reward += rollout.get_reward();
        }
        return total_reward / num_rollouts;
    }

    ActionIdxT random_policy(Game& game_state){
        return rand() % game_state.num_actions;
    }

    MCTSNode(MCTSNode* parent){
        this->parent = parent;
        this->n_visits = 0;
        this->Q = 0;
        this->is_expanded = false;
        if(parent != nullptr){
            this->player = parent->player;
        }
    }

    void delete_rec(){
        for (ActionIdxT i = 0; i < num_actions; i++){
            if(children[i] != nullptr) children[i]->delete_rec();
        }
        delete this;
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
                // If child is empty, set up data structure
                if (child == nullptr){
                    children[i] = new MCTSNode(this);
                    child = children[i];
                }
                if (child->n_visits == 0){
                    num_explored_actions++;
                    return std::make_pair(child, i);
                }
            }
        }

        // Select child with highest UCB - simple scan
        double best_uct = -INF;
        MCTSNode<Game>* best_child = nullptr;
        ActionIdxT best_action = -1;
        // Iterate over pairs of children and actions
        // The actions of the game state match with the MCTSNode children
        for (ActionIdxT i = 0; i < num_actions; i++){
            MCTSNode<Game>* child = children[i];
            double uct = child->Q + sqrt(2 * log(n_visits) / child->n_visits);
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

    void traverse(int num_iters, Game& game_state){
        for (int i = 0; i < num_iters; i++){
            MCTSNode<Game>* node = this;
            Game state = game_state.copy();
            while (node->is_expanded){
                auto [child, action] = node->select_child();
                state.step(action);
                node = child;
            }
            RewardT reward = node->expand(state);
            node->update_recursive(reward);
        }
    }

    void print(){
        std::cout << "Q: " << Q << std::endl;
        std::cout << "n_visits: " << n_visits << std::endl;
        std::cout << "num_actions: " << num_actions << std::endl;
        std::cout << "num_explored_actions: " << num_explored_actions << std::endl;
        std::cout << "is_expanded: " << is_expanded << std::endl;
    }
};

void run_sim(){
    constexpr int BOARD_SIZE = 8;
    using Game = Connect4<BOARD_SIZE>;
    int MAX_PLY = BOARD_SIZE * BOARD_SIZE;

    // using Game = TicTacToe;
    // int MAX_PLY = 9;

    Game game;
    int num_iters = 10000;

    for (int num_ply = 0; num_ply < MAX_PLY; num_ply++){
        MCTSNode<Game>* root = new MCTSNode<Game>(nullptr);
        root->player = game.player;
        if(game.is_terminal()){
            break;
        }
        std::cout << "Ply: " << num_ply << std::endl;
        root->traverse(num_iters, game);
        auto [best_child, best_action] = root->most_visited();
        game.step(best_action);
        std::cout << "Best action: " << best_action << std::endl;
        best_child->print();
        game.print();
        root->delete_rec();
    }
}

void play_game(){
    TicTacToe game;
    MCTSNode<TicTacToe>* root = new MCTSNode<TicTacToe>(nullptr);
    int num_iters = 1000;

    for (int num_ply = 0; num_ply < 9; num_ply += 2){
        std::cout << "Ply: " << num_ply << std::endl;

        std::cout << "Enter your move" << std::endl;
        game.print();
        int action;
        std::cin >> action;
        game.step(action);
        
        root->traverse(num_iters, game);
        auto [best_child, best_action] = root->most_visited();
        game.step(best_action);
        best_child->make_root();
        std::cout << "Best action: " << best_action << std::endl;
        root->print();
        game.print();
        root = best_child;
    }
}

int main(){
    run_sim();
    // play_game();
}
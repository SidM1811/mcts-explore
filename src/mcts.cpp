#include <vector>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <fstream>

#include <thread>
#include <vector>

// Include the Game class header
#include "game_dynamics/tictactoe.hpp"
#include "game_dynamics/connect4.hpp"

#include "batch_malloc.hpp"
#include "thread_safe_batch_malloc.hpp"

#include "game_net/connect4_hf.hpp"

#define NUM_ROLLOUTS 10

template <typename Game>
class MCTSNode {
    constexpr static double INF = 1e6;
    public:
    using ActionT = typename Game::ActionT;
    using ActionIdxT = int;
    using RewardT = typename Game::RewardT;
    using PlayerType = typename Game::PlayerType;

    using Net = HF_Net<8>;

    MCTSNode<Game>* parent;
    std::vector<MCTSNode<Game>*> children;
    int n_visits;
    double Q;
    bool is_expanded;
    int num_actions, num_explored_actions;
    PlayerType player;

    using AllocT = ThreadSafeBatchMalloc<MCTSNode<Game>>;

    static AllocT* allocator;
    static Net* hf_net;
    
    // Static method to get/initialize allocator
    static AllocT* get_allocator() {
        if (allocator == nullptr) {
            allocator = new AllocT(10000000);
        }
        return allocator;
    }

    static Net* get_hf_net() {
        if (hf_net == nullptr) {
            hf_net = new Net();
        }
        return hf_net;
    }

    RewardT net_rollout(Game& game_state){
        return get_hf_net()->forward(game_state);
    }

    RewardT random_rollouts(Game& game_state, int num_rollouts){
        RewardT total_reward = {0, 0};
        Game rollout;
        for (int i = 0; i < num_rollouts; i++){
            game_state.copy(rollout);
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
        if (is_expanded) {
            for (ActionIdxT i = 0; i < num_actions; i++){
                if (children[i] != nullptr) {
                    children[i]->delete_rec();
                }
            }
        }
        get_allocator()->push(reinterpret_cast<MCTSNode<Game>*>(this));
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

    std::pair<MCTSNode*, ActionIdxT> soft_select(){
        std::vector<double> action_probs(num_actions, 0.0);
        double sum_probs = 0.0;
        double dirichlet_alpha = 0.3;

        // Calculate softmax probabilities based on Q values
        for (ActionIdxT i = 0; i < num_actions; i++){
            MCTSNode* child = children[i];
            if (child != nullptr){
                action_probs[i] = child->n_visits + dirichlet_alpha;
                sum_probs += action_probs[i];
            }
        }

        // Normalize probabilities
        for (ActionIdxT i = 0; i < num_actions; i++){
            action_probs[i] /= sum_probs;
        }

        // Select action based on probabilities
        double rand_val = static_cast<double>(rand()) / RAND_MAX;
        double cumulative_prob = 0.0;
        for (ActionIdxT i = 0; i < num_actions; i++){
            cumulative_prob += action_probs[i];
            if (rand_val < cumulative_prob){
                return std::make_pair(children[i], i);
            }
        }
        return std::make_pair(nullptr, -1); // Should not reach here
    }

    std::pair<MCTSNode*, ActionIdxT> select_child(){
        if (num_explored_actions < num_actions){
            // Select unexplored child - should be uniform but here deterministic
            for (ActionIdxT i = 0; i < num_actions; i++){
                MCTSNode* child = children[i];
                // If child is empty, set up data structure
                if (child == nullptr){
                    children[i] = get_allocator()->pop();
                    new (children[i]) MCTSNode<Game>(this);
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
        Game state;
        for (int i = 0; i < num_iters; i++){
            MCTSNode<Game>* node = this;
            game_state.copy(state);
            while (node->is_expanded){
                auto [child, action] = node->select_child();
                state.step(action);
                node = child;
            }
            // Init Q with neural network
            node->Q = get_hf_net()->forward(state)[player];
            node->n_visits = 1;
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

// Static member definitions
template <typename Game>
typename MCTSNode<Game>::AllocT* MCTSNode<Game>::allocator = nullptr;

template <typename Game>
typename MCTSNode<Game>::Net* MCTSNode<Game>::hf_net = nullptr;

void run_sim(){
    constexpr int BOARD_SIZE = 8;
    using Game = Connect4<BOARD_SIZE>;
    int MAX_PLY = BOARD_SIZE * BOARD_SIZE;

    Game PV[MAX_PLY];

    Game game;
    int num_iters = 100000;
    int num_ply = 0;
    MCTSNode<Game>* root = MCTSNode<Game>::get_allocator()->pop();
    for (num_ply = 0; num_ply < MAX_PLY; num_ply++){
        if (root != nullptr) {
            root->delete_rec();
        }
        // Allocate memory
        root = MCTSNode<Game>::get_allocator()->pop();
        // Initialize
        new (root) MCTSNode<Game>(nullptr);
        root->player = game.player;
        if(game.is_terminal()){
            break;
        }
        std::cout << "Ply: " << num_ply << std::endl;
        root->traverse(num_iters, game);
        auto [best_child, best_action] = root->soft_select();
        std::cout << "Best action: " << game.action_map[best_action] << std::endl;
        game.copy(PV[num_ply]);
        game.step(best_action);
        best_child->print();
        game.print();
    }
    int result = game.get_reward()[0];
    
    // Get the directory where the executable is located
    std::string data_file = "../data/game_data.csv";

    double evals[MCTSNode<Game>::Net::NUM_FEATURES];
    
    for (int i = 0; i < num_ply; i++){
        // Save game to file as csv
        std::ofstream file(data_file, std::ios::app);
        if (file.is_open()) {
            root->get_hf_net()->fill_evals(PV[i], evals);
            for (int j = 0; j < MCTSNode<Game>::Net::NUM_FEATURES; j++){
                file << evals[j] << ",";
            }
            file << result << std::endl;
            file.close();
        } else {
            std::cerr << "Error: Could not open file " << data_file << " for appending move " << i << std::endl;
        }
    }
    std::cout << "Game data saved to " << data_file << std::endl;
    if(root != nullptr) root->delete_rec();
}

int main(){
    constexpr int NUM_CORES = 20;
    constexpr int NUM_GAMES = 10 * NUM_CORES;
    
    std::vector<std::thread> threads;
    
    for (int core = 0; core < NUM_CORES; core++) {
        threads.emplace_back([core]() {
            // Run games on this core
            for (int i = core; i < NUM_GAMES; i += NUM_CORES) {
                run_sim();
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
}
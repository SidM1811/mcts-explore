#ifndef CONNECT4_NET_HPP
#define CONNECT4_NET_HPP
#include "../game_dynamics/connect4.hpp"
#include <torch/torch.h> 

#define HIDDEN_DIM 128

template<int BOARD_SIZE>
class Connect4Net : public torch::nn::Module {
    public:
    Connect4Net() {
        // Initialize the neural network model
        model = torch::nn::Sequential(
            torch::nn::Linear(BOARD_SIZE * BOARD_SIZE * 2, HIDDEN_DIM),
            torch::nn::ReLU(),
            torch::nn::Linear(HIDDEN_DIM, HIDDEN_DIM),
            torch::nn::ReLU(),
            torch::nn::Linear(HIDDEN_DIM, BOARD_SIZE + 1),
        );
    }

    torch::Tensor forward(torch::Tensor x) {
        return model->forward(x);
    }

    torch::Tensor create_input(Connect4<BOARD_SIZE>& game){
        // Create a tensor of shape (BOARD_SIZE * BOARD_SIZE * 2)
        torch::Tensor input = torch::zeros({BOARD_SIZE * BOARD_SIZE * 2}, torch::kFloat32);
        // Fill the tensor with the game state
        for (int player = 0; player < 2; player++) {
            for(int i = 0; i < BOARD_SIZE; i++){
                for(int j = 0; j < BOARD_SIZE; j++){
                    if(is_set(player, i, j, game.state)){
                        // Set the corresponding position in the tensor
                        input[(player * BOARD_SIZE * BOARD_SIZE) + (i * BOARD_SIZE) + j] = 1.0;
                    } else {
                        // Set the corresponding position in the tensor to 0
                        input[(player * BOARD_SIZE * BOARD_SIZE) + (i * BOARD_SIZE) + j] = 0.0;
                    }
                }
            }
        }
        return input;
    }

    private:
    torch::nn::Sequential model;
};
#endif
#include <cmath>
#include "utils.h"

#define NUM_SQUARES 64
#define NUM_PLAYERS 2
#define NUM_PIECES 5 // Except the king

enum PieceType {
    PAWN = 0,
    KNIGHT = 1,
    BISHOP = 2,
    ROOK = 3,
    QUEEN = 4,
    KING = 5
};

struct BitBoard {
    uint64_t board[NUM_SQUARES];
};

template <typename DataT, int M, int N>
class ActivationsT {
    public:
    DataT data[M][N];
};

template <typename DataT, int M, int N>
class LinearLayer {
    public:
    DataT weights[N][M];
    DataT bias[N];

    void forward(DataT input[M], DataT output[N]) {
        matmul<DataT, N, M, M>(weights, input, output);
        add<DataT, N, 1>(output, bias);
    }
};


template <typename WeightT>
class EvalState {
    // Weights for players
    LinearLayer<WeightT, NUM_SQUARES * NUM_PIECES * NUM_SQUARES * NUM_PLAYERS, 256> linear1[NUM_PLAYERS];
    LinearLayer<WeightT, 256 * 2, 256> linear2;
    LinearLayer<WeightT, 256, 1> linear3;
    public: 
    // Represents input state with one-hot encoding
    BitBoard state[NUM_PLAYERS][NUM_SQUARES][NUM_PIECES][NUM_PLAYERS]; // self, king square, piece type
    ActivationsT<WeightT, 256, 1> hidden1[NUM_PLAYERS];
    ActivationsT<WeightT, 256 * 2, 1> hidden1_concat;
    ActivationsT<WeightT, 256, 1> hidden2;

    bool flip = false;
    

    WeightT forward() {
        WeightT input[NUM_PLAYERS][NUM_SQUARES * NUM_PIECES * NUM_SQUARES * NUM_PLAYERS];
        for (int player = 0; player < NUM_PLAYERS; player++) {
            for (int square = 0; square < NUM_SQUARES; square++) {
                for (int piece = 0; piece < NUM_PIECES; piece++) {
                    for (int other_square = 0; other_square < NUM_SQUARES; other_square++) {
                        for (int other_player = 0; other_player < NUM_PLAYERS; other_player++) {
                            int idx = square * NUM_PIECES * NUM_SQUARES * NUM_PLAYERS + piece * NUM_SQUARES * NUM_PLAYERS + other_square * NUM_PLAYERS + other_player;
                            input[player][idx] = state[player][square][piece][other_player].board[other_square];
                        }
                    }
                }
            }
        }

        WeightT result;
        linear1[0].forward(input[0], hidden1[0].data);
        linear1[1].forward(input[1], hidden1[1].data);
        concat<WeightT, 256, 256, 1>(hidden1[0].data, hidden1[1].data, hidden1_concat.data);
        ReLU<WeightT, 256 * 2, 1>(hidden1_concat.data);
        linear2.forward(hidden1_concat.data, hidden2.data);
        ReLU<WeightT, 256, 1>(hidden2.data);
        linear3.forward(hidden2.data, &result);
        return result;
    }
};

int main() {
    EvalState<float> state;
    state.forward();
    return 0;
}
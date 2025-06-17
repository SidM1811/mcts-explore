#ifndef CONNECT4_H
#define CONNECT4_H

#include <vector>
#include <map>
#include <array>
#include <cstring> // For memcpy
#include <iostream>
#include <cassert>

#define round_up(x, y) (((x) + (y) - 1) / (y))

template <int BOARD_SIZE>
class Connect4 {
public:
    using ActionT = int; // Action is just the column index
    using RewardT = std::array<double, 2>;
    using BoardRepT = uint8_t;
    constexpr static int BOARD_REP_SIZE = sizeof(BoardRepT) * 8;
    BoardRepT state[2][BOARD_SIZE][round_up(BOARD_SIZE, BOARD_REP_SIZE)];
    enum PlayerType{
        Empty = -1,
        Player0 = 0,
        Player1 = 1
    };
    PlayerType player;
    int num_actions;
    int col_heights[BOARD_SIZE];
    ActionT action_map[BOARD_SIZE];
    int last_row, last_col; // Track last move for optimization

    inline PlayerType get_next_player() {
        return PlayerType(player ^ 1);
    }

    inline PlayerType get_prev_player() {
        return PlayerType(player ^ 1);
    }

    Connect4() {
        // Initialize board to empty
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < round_up(BOARD_SIZE, BOARD_REP_SIZE); j++) {
                state[Player0][i][j] = static_cast<BoardRepT>(0);
                state[Player1][i][j] = static_cast<BoardRepT>(0);
            }
            col_heights[i] = 0;
            action_map[i] = i; // Initialize action map with column indices
        }
        player = Player0;
        num_actions = BOARD_SIZE;
    }

    Connect4 copy(){     
        Connect4 copy_game;   
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < round_up(BOARD_SIZE, BOARD_REP_SIZE); j++) {
                copy_game.state[Player0][i][j] = this->state[Player0][i][j];
                copy_game.state[Player1][i][j] = this->state[Player1][i][j];
            }
            copy_game.col_heights[i] = this->col_heights[i];
            copy_game.action_map[i] = this->action_map[i];
        }
        
        // Copy other members
        copy_game.player = this->player;
        copy_game.num_actions = this->num_actions;
        copy_game.last_row = this->last_row;
        copy_game.last_col = this->last_col;
        
        return copy_game;
    }

    int get_num_actions() {
        return num_actions;
    }

    // Mutates the object!!
    void step(int action_idx) {
        int col = action_map[action_idx];
        int row = col_heights[col]++;
        state[player][col][row / BOARD_REP_SIZE] |= (static_cast<BoardRepT>(1) << (row % BOARD_REP_SIZE));
        
        // If column is full, remove it from action_map efficiently
        if(col_heights[col] >= BOARD_SIZE){
            // Swap with last element and reduce size - O(1) instead of O(n)
            action_map[action_idx] = action_map[num_actions - 1];
            num_actions--;
        }
        
        last_row = row;
        last_col = col;
        player = get_next_player();
    }

    inline bool is_set(PlayerType player, int row, int col){
        return (
            state[player][col][row / BOARD_REP_SIZE] 
            >> (row % BOARD_REP_SIZE)
        ) & 1;
    }

    bool is_winner(PlayerType check_player){
        // Check all positions for 4-in-a-row in all directions
        // if(last_row < 0 || last_col < 0 || last_row >= BOARD_SIZE || last_col >= BOARD_SIZE) {
        //     return false; // Invalid last move
        // }
        for (int col = 0; col < BOARD_SIZE; col++) {
            for (int row = 0; row < BOARD_SIZE; row++) {
                if (is_set(check_player, row, col)) {
                    // Check horizontal (right)
                    if (col + 3 < BOARD_SIZE &&
                        is_set(check_player, row, col + 1) &&
                        is_set(check_player, row, col + 2) &&
                        is_set(check_player, row, col + 3)) {
                        return true;
                    }
                    
                    // Check vertical (down)
                    if (row + 3 < BOARD_SIZE &&
                        is_set(check_player, row + 1, col) &&
                        is_set(check_player, row + 2, col) &&
                        is_set(check_player, row + 3, col)) {
                        return true;
                    }
                    
                    // Check diagonal (down-right)
                    if (row + 3 < BOARD_SIZE && col + 3 < BOARD_SIZE &&
                        is_set(check_player, row + 1, col + 1) &&
                        is_set(check_player, row + 2, col + 2) &&
                        is_set(check_player, row + 3, col + 3)) {
                        return true;
                    }
                    
                    // Check anti-diagonal (down-left)
                    if (row + 3 < BOARD_SIZE && col - 3 >= 0 &&
                        is_set(check_player, row + 1, col - 1) &&
                        is_set(check_player, row + 2, col - 2) &&
                        is_set(check_player, row + 3, col - 3)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool is_terminal(){
        if (is_winner(Player0) || is_winner(Player1) || num_actions == 0){
            return true;
        }
        return false;
    }

    RewardT get_reward(){
        assert(is_terminal());
        if (is_winner(Player0)){
            return RewardT{1, -1};
        } else if (is_winner(Player1)){
            return RewardT{-1, 1};
        } else {
            return RewardT{0, 0};
        }
    }

    void print(){
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (is_set(Player0, i, j)){
                    std::cout << "X";
                } else if (is_set(Player1, i, j)){
                    std::cout << "O";
                } else {
                    std::cout << ".";
                }
            }
            std::cout << std::endl;
        }
    }
};

template <int BOARD_SIZE>
typename Connect4<BOARD_SIZE>::RewardT& operator +=(typename Connect4<BOARD_SIZE>::RewardT& lhs, const typename Connect4<BOARD_SIZE>::RewardT& rhs) {
    lhs[0] += rhs[0];
    lhs[1] += rhs[1];
    return lhs;
}

template <int BOARD_SIZE>
typename Connect4<BOARD_SIZE>::RewardT operator /(const typename Connect4<BOARD_SIZE>::RewardT& lhs, int rhs) {
    return typename Connect4<BOARD_SIZE>::RewardT{lhs[0] / rhs, lhs[1] / rhs};
}

#endif
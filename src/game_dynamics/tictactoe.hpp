#ifndef TICTACTOE_H
#define TICTACTOE_H

#include <vector>
#include <map>
#include <array>

class TicTacToe {
public:
    using ActionT = std::pair<int, int> ;
    using RewardT = std::array<double, 2>;
    enum PlayerType{
        Empty = -1,
        Player0 = 0,
        Player1 = 1
    };
    int state[3][3];
    PlayerType player;
    int num_actions;
    std::vector<ActionT> action_map;

    __attribute__((always_inline)) inline PlayerType get_next_player() {
        return PlayerType(player ^ 1);
    }

    __attribute__((always_inline)) inline PlayerType get_prev_player() {
        return PlayerType(player ^ 1);
    }

    TicTacToe() {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                state[i][j] = Empty;
                action_map.push_back(std::make_pair(i, j));
            };
        }
        player = Player0;
        num_actions = 9;
    }

    TicTacToe copy(TicTacToe& copy) {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                copy.state[i][j] = this->state[i][j];
            };
        }
        copy.player = this->player;
        copy.num_actions = this->num_actions;
        copy.action_map = this->action_map;
        return copy;
    }

    int get_num_actions() {
        return num_actions;
    }

    // Mutates the object!!
    void step(int action_idx) {
        auto [row, col] = action_map[action_idx];
        // Slow!!
        action_map.erase(action_map.begin() + action_idx);
        state[row][col] = player;
        player = get_next_player();
        num_actions--;
    }

    bool is_winner(PlayerType player){
        for (int i = 0; i < 3; i++) {
            if (state[i][0] == player && state[i][1] == player && state[i][2] == player) {
                return true;
            }
            if (state[0][i] == player && state[1][i] == player && state[2][i] == player) {
                return true;
            }
        }
        if (state[0][0] == player && state[1][1] == player && state[2][2] == player) {
            return true;
        }
        if (state[0][2] == player && state[1][1] == player && state[2][0] == player) {
            return true;
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
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (state[i][j] == Player0){
                    std::cout << "X";
                } else if (state[i][j] == Player1){
                    std::cout << "O";
                } else {
                    std::cout << ".";
                }
            }
            std::cout << std::endl;
        }
    }
};

TicTacToe::RewardT& operator +=(TicTacToe::RewardT& lhs, const TicTacToe::RewardT& rhs) {
    lhs[0] += rhs[0];
    lhs[1] += rhs[1];
    return lhs;
}

TicTacToe::RewardT operator /(const TicTacToe::RewardT& lhs, int rhs) {
    return TicTacToe::RewardT{lhs[0] / rhs, lhs[1] / rhs};
}

#endif
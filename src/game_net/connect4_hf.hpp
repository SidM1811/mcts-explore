#ifndef CONNECT4_HF_HPP
#define CONNECT4_HF_HPP
#include "../game_dynamics/connect4.hpp"
#include <algorithm>

template<int BOARD_SIZE>
class HF_Net{
    using Game = Connect4<BOARD_SIZE>;
    using RewardT = typename Game::RewardT;
    using PlayerType = typename Game::PlayerType;
    public:
    constexpr static int NUM_FEATURES = 15;
    double weights[NUM_FEATURES] = {
        0.01238058,
        -0.00597948,
        0.09921825,
        0.12328073,
        -0.23779304,
        0.19077861,
        -4.97728920,
        -0.15271282,
        -0.07976994,
        0.58472133,
        0.08261606,
        -0.04234111,
        2.28158832,
        0.00706849,
        -0.69452399
    };

    double (HF_Net::*fptr[NUM_FEATURES]) (const Game& game);
    
    HF_Net(){
        fptr[0] = &HF_Net::open3_feature;
        fptr[1] = &HF_Net::open2_feature;
        fptr[2] = &HF_Net::threat_feature;
        fptr[3] = &HF_Net::center_control_feature;
        fptr[4] = &HF_Net::blocking_feature;
        fptr[5] = &HF_Net::height_advantage_feature;
        fptr[6] = &HF_Net::connectivity_feature;
        fptr[7] = &HF_Net::fork_feature;
        fptr[8] = &HF_Net::tempo_feature;
        fptr[9] = &HF_Net::edge_avoidance_feature;
        fptr[10] = &HF_Net::trap_feature;
        fptr[11] = &HF_Net::mobility_feature;
        fptr[12] = &HF_Net::structure_feature;
        fptr[13] = &HF_Net::defensive_pattern_feature;
        fptr[14] = &HF_Net::endgame_feature;
    }

    RewardT forward(const Game& game) {
        double reward = 0.0;

        for(int i = 0; i < NUM_FEATURES; i++){
            reward += (this->*fptr[i])(game) * weights[i];
        }

        // Apply tanh to squash reward between -1 and 1
        reward = std::tanh(reward);
        return RewardT{reward, -reward};
    }

    void* fill_evals(const Game& game, double* arr){
        for(int i = 0; i < NUM_FEATURES; i++){
            arr[i] = (this->*fptr[i])(game);
        }
    }

    double open3_helper(const Game& game, PlayerType check_player, int row, int col) {
        double open3 = 0.0;
        PlayerType other_player = PlayerType(check_player ^ 1);

        // Check horizontal - all possible 3-in-a-row that include this position
        for (int start_col = std::max(0, col - 3); start_col <= std::min(col, BOARD_SIZE - 4); start_col++) {
            if (game.is_set(check_player, row, start_col) &&
                game.is_set(check_player, row, start_col + 1) &&
                game.is_set(check_player, row, start_col + 2) &&
                !game.is_set(other_player, row, start_col + 3)) {
                open3 += 1.0;
            }
        }
        
        // Check vertical - all possible 3-in-a-row that include this position
        for (int start_row = std::max(0, row - 3); start_row <= std::min(row, BOARD_SIZE - 4); start_row++) {
            if (game.is_set(check_player, start_row, col) &&
                game.is_set(check_player, start_row + 1, col) &&
                game.is_set(check_player, start_row + 2, col) &&
                !game.is_set(other_player, start_row + 3, col)) {
                open3 += 1.0;
            }
        }
        
        // Check diagonal (top-left to bottom-right)
        for (int i = -3; i <= 0; i++) {
            int start_row = row + i;
            int start_col = col + i;
            if (start_row >= 0 && start_col >= 0 && 
                start_row + 3 < BOARD_SIZE && start_col + 3 < BOARD_SIZE) {
                if (game.is_set(check_player, start_row, start_col) &&
                    game.is_set(check_player, start_row + 1, start_col + 1) &&
                    game.is_set(check_player, start_row + 2, start_col + 2) &&
                    !game.is_set(other_player, start_row + 3, start_col + 3)) {
                    open3 += 1.0;
                }
            }
        }
        
        // Check anti-diagonal (top-right to bottom-left)
        for (int i = -3; i <= 0; i++) {
            int start_row = row + i;
            int start_col = col - i;
            if (start_row >= 0 && start_col < BOARD_SIZE && 
                start_row + 3 < BOARD_SIZE && start_col - 3 >= 0) {
                if (game.is_set(check_player, start_row, start_col) &&
                    game.is_set(check_player, start_row + 1, start_col - 1) &&
                    game.is_set(check_player, start_row + 2, start_col - 2) &&
                    !game.is_set(other_player, start_row + 3, start_col - 3)) {
                    open3 += 1.0;
                }
            }
        }
        return open3;
    }

    double open3_feature(const Game& game) {
        double total_open3 = 0.0;
        for (int row = 0; row < BOARD_SIZE; row++) {
            for (int col = 0; col < BOARD_SIZE; col++) {
                if (game.is_set(PlayerType::Player0, row, col)) {
                    total_open3 += open3_helper(game, PlayerType::Player0, row, col);
                } else if (game.is_set(PlayerType::Player1, row, col)) {
                    total_open3 -= open3_helper(game, PlayerType::Player1, row, col);
                }
            }
        }
        return total_open3;
    }

    // Helper function for 2-in-a-row with open ends
    double open2_helper(const Game& game, PlayerType check_player, int row, int col) {
        double open2 = 0.0;
        PlayerType other_player = PlayerType(check_player ^ 1);

        // Check horizontal 2-in-a-row patterns
        for (int start_col = std::max(0, col - 2); start_col <= std::min(col, BOARD_SIZE - 3); start_col++) {
            if (start_col + 2 < BOARD_SIZE &&
                game.is_set(check_player, row, start_col) &&
                game.is_set(check_player, row, start_col + 1) &&
                !game.is_set(other_player, row, start_col) &&
                !game.is_set(other_player, row, start_col + 1)) {
                
                // Check if there's space to extend on either side
                bool left_open = (start_col > 0) && !game.is_set(other_player, row, start_col - 1);
                bool right_open = (start_col + 2 < BOARD_SIZE - 1) && !game.is_set(other_player, row, start_col + 2);
                
                if (left_open || right_open) {
                    open2 += 0.5;
                }
            }
        }
        
        // Check vertical 2-in-a-row patterns
        for (int start_row = std::max(0, row - 2); start_row <= std::min(row, BOARD_SIZE - 3); start_row++) {
            if (start_row + 2 < BOARD_SIZE &&
                game.is_set(check_player, start_row, col) &&
                game.is_set(check_player, start_row + 1, col) &&
                !game.is_set(other_player, start_row, col) &&
                !game.is_set(other_player, start_row + 1, col)) {
                
                bool top_open = (start_row > 0) && !game.is_set(other_player, start_row - 1, col);
                bool bottom_open = (start_row + 2 < BOARD_SIZE - 1) && !game.is_set(other_player, start_row + 2, col);
                
                if (top_open || bottom_open) {
                    open2 += 0.5;
                }
            }
        }
        
        // Check diagonal patterns (simplified for performance)
        for (int i = -2; i <= 0; i++) {
            int start_row = row + i;
            int start_col = col + i;
            if (start_row >= 0 && start_col >= 0 && 
                start_row + 2 < BOARD_SIZE && start_col + 2 < BOARD_SIZE) {
                if (game.is_set(check_player, start_row, start_col) &&
                    game.is_set(check_player, start_row + 1, start_col + 1)) {
                    open2 += 0.3;
                }
            }
        }
        
        return open2;
    }

    double open2_feature(const Game& game) {
        double total_open2 = 0.0;
        for (int row = 0; row < BOARD_SIZE; row++) {
            for (int col = 0; col < BOARD_SIZE; col++) {
                if (game.is_set(PlayerType::Player0, row, col)) {
                    total_open2 += open2_helper(game, PlayerType::Player0, row, col);
                } else if (game.is_set(PlayerType::Player1, row, col)) {
                    total_open2 -= open2_helper(game, PlayerType::Player1, row, col);
                }
            }
        }
        return total_open2;
    }

    // Detect immediate winning threats (3-in-a-row that can be completed)
    double threat_feature(const Game& game) {
        double threats = 0.0;
        
        for (int row = 0; row < BOARD_SIZE; row++) {
            for (int col = 0; col < BOARD_SIZE; col++) {
                // Check if this position is empty and can create a winning threat
                if (!game.is_set(PlayerType::Player0, row, col) && 
                    !game.is_set(PlayerType::Player1, row, col)) {
                    
                    // Check if placing Player0 here creates a win
                    if (would_create_win(game, PlayerType::Player0, row, col)) {
                        threats += 1.0;
                    }
                    // Check if placing Player1 here creates a win
                    if (would_create_win(game, PlayerType::Player1, row, col)) {
                        threats -= 1.0;
                    }
                }
            }
        }
        return threats;
    }

    // Helper to check if placing a piece would create a winning line
    bool would_create_win(const Game& game, PlayerType player, int row, int col) {
        // Temporarily check if placing piece here would create 4-in-a-row
        
        // Horizontal check
        for (int start_col = std::max(0, col - 3); start_col <= std::min(col, BOARD_SIZE - 4); start_col++) {
            int count = 0;
            for (int c = start_col; c <= start_col + 3; c++) {
                if (c == col || game.is_set(player, row, c)) {
                    count++;
                }
            }
            if (count == 4) return true;
        }
        
        // Vertical check
        for (int start_row = std::max(0, row - 3); start_row <= std::min(row, BOARD_SIZE - 4); start_row++) {
            int count = 0;
            for (int r = start_row; r <= start_row + 3; r++) {
                if (r == row || game.is_set(player, r, col)) {
                    count++;
                }
            }
            if (count == 4) return true;
        }
        
        // Diagonal checks (simplified)
        for (int i = -3; i <= 0; i++) {
            int start_row = row + i;
            int start_col = col + i;
            if (start_row >= 0 && start_col >= 0 && 
                start_row + 3 < BOARD_SIZE && start_col + 3 < BOARD_SIZE) {
                int count = 0;
                for (int j = 0; j <= 3; j++) {
                    int r = start_row + j;
                    int c = start_col + j;
                    if ((r == row && c == col) || game.is_set(player, r, c)) {
                        count++;
                    }
                }
                if (count == 4) return true;
            }
        }
        
        return false;
    }

    // Center control - pieces in center columns are more valuable
    double center_control_feature(const Game& game) {
        double center_score = 0.0;
        int center_start = BOARD_SIZE / 2 - 1;
        int center_end = BOARD_SIZE / 2 + 1;
        
        for (int row = 0; row < BOARD_SIZE; row++) {
            for (int col = center_start; col <= center_end && col < BOARD_SIZE; col++) {
                if (game.is_set(PlayerType::Player0, row, col)) {
                    center_score += 0.5;
                } else if (game.is_set(PlayerType::Player1, row, col)) {
                    center_score -= 0.5;
                }
            }
        }
        return center_score;
    }

    // Blocking feature - reward blocking opponent's threats
    double blocking_feature(const Game& game) {
        double blocking_score = 0.0;
        
        for (int row = 0; row < BOARD_SIZE; row++) {
            for (int col = 0; col < BOARD_SIZE; col++) {
                if (game.is_set(PlayerType::Player0, row, col)) {
                    // Check if this piece blocks Player1's potential lines
                    blocking_score += count_blocked_lines(game, PlayerType::Player1, row, col);
                } else if (game.is_set(PlayerType::Player1, row, col)) {
                    blocking_score -= count_blocked_lines(game, PlayerType::Player0, row, col);
                }
            }
        }
        return blocking_score;
    }

    // Count how many opponent lines this position blocks
    double count_blocked_lines(const Game& game, PlayerType opponent, int row, int col) {
        double blocked = 0.0;
        
        // Check horizontal lines this position could block
        for (int start_col = std::max(0, col - 3); start_col <= std::min(col, BOARD_SIZE - 4); start_col++) {
            int opponent_count = 0;
            for (int c = start_col; c <= start_col + 3; c++) {
                if (c != col && game.is_set(opponent, row, c)) {
                    opponent_count++;
                }
            }
            if (opponent_count >= 2) blocked += 0.3;
        }
        
        // Similar checks for vertical and diagonal (simplified for performance)
        return blocked;
    }

    // Height advantage - prefer playing in lower rows (gravity simulation)
    double height_advantage_feature(const Game& game) {
        double height_score = 0.0;
        
        for (int row = 0; row < BOARD_SIZE; row++) {
            double row_weight = (BOARD_SIZE - row) * 0.1; // Lower rows have higher weight
            for (int col = 0; col < BOARD_SIZE; col++) {
                if (game.is_set(PlayerType::Player0, row, col)) {
                    height_score += row_weight;
                } else if (game.is_set(PlayerType::Player1, row, col)) {
                    height_score -= row_weight;
                }
            }
        }
        return height_score;
    }

    // Connectivity feature - reward pieces that are connected to other pieces
    double connectivity_feature(const Game& game) {
        double connectivity = 0.0;
        
        for (int row = 0; row < BOARD_SIZE; row++) {
            for (int col = 0; col < BOARD_SIZE; col++) {
                if (game.is_set(PlayerType::Player0, row, col)) {
                    connectivity += count_adjacent_pieces(game, PlayerType::Player0, row, col);
                } else if (game.is_set(PlayerType::Player1, row, col)) {
                    connectivity -= count_adjacent_pieces(game, PlayerType::Player1, row, col);
                }
            }
        }
        return connectivity;
    }

    // Count adjacent pieces of the same player
    double count_adjacent_pieces(const Game& game, PlayerType player, int row, int col) {
        double adjacent = 0.0;
        
        // Check all 8 directions
        int directions[8][2] = {{-1,-1}, {-1,0}, {-1,1}, {0,-1}, {0,1}, {1,-1}, {1,0}, {1,1}};
        
        for (int i = 0; i < 8; i++) {
            int new_row = row + directions[i][0];
            int new_col = col + directions[i][1];
            
            if (new_row >= 0 && new_row < BOARD_SIZE && 
                new_col >= 0 && new_col < BOARD_SIZE &&
                game.is_set(player, new_row, new_col)) {
                adjacent += 0.1;
            }
        }
        return adjacent;
    }

    // Fork feature - detect positions that create multiple winning threats
    double fork_feature(const Game& game) {
        double fork_score = 0.0;
        
        for (int row = 0; row < BOARD_SIZE; row++) {
            for (int col = 0; col < BOARD_SIZE; col++) {
                if (!game.is_set(PlayerType::Player0, row, col) && 
                    !game.is_set(PlayerType::Player1, row, col)) {
                    
                    // Check how many winning threats this move would create
                    int p0_threats = count_threats_created(game, PlayerType::Player0, row, col);
                    int p1_threats = count_threats_created(game, PlayerType::Player1, row, col);
                    
                    if (p0_threats >= 2) fork_score += p0_threats * 0.8;
                    if (p1_threats >= 2) fork_score -= p1_threats * 0.8;
                }
            }
        }
        return fork_score;
    }

    // Count how many winning threats placing a piece would create
    int count_threats_created(const Game& game, PlayerType player, int row, int col) {
        int threats = 0;
        
        // Check all directions for potential 3-in-a-row that would become threats
        int directions[4][2] = {{0,1}, {1,0}, {1,1}, {1,-1}};
        
        for (int d = 0; d < 4; d++) {
            int dr = directions[d][0];
            int dc = directions[d][1];
            
            // Check both directions along this line
            for (int dir = -1; dir <= 1; dir += 2) {
                int count = 1; // Count the piece we're placing
                int spaces = 0;
                
                // Count pieces in one direction
                for (int i = 1; i <= 3; i++) {
                    int nr = row + dir * i * dr;
                    int nc = col + dir * i * dc;
                    
                    if (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE) {
                        if (game.is_set(player, nr, nc)) {
                            count++;
                        } else if (!game.is_set(PlayerType(player ^ 1), nr, nc)) {
                            spaces++;
                        } else {
                            break;
                        }
                    }
                }
                
                // If we have 3 pieces with space for a 4th, it's a threat
                if (count == 3 && spaces >= 1) {
                    threats++;
                }
            }
        }
        return threats;
    }

    // Tempo feature - reward forcing moves that limit opponent's options
    double tempo_feature(const Game& game) {
        double tempo_score = 0.0;
        
        // Count immediate threats for each player
        int p0_immediate_threats = 0;
        int p1_immediate_threats = 0;
        
        for (int row = 0; row < BOARD_SIZE; row++) {
            for (int col = 0; col < BOARD_SIZE; col++) {
                if (!game.is_set(PlayerType::Player0, row, col) && 
                    !game.is_set(PlayerType::Player1, row, col)) {
                    
                    if (would_create_win(game, PlayerType::Player0, row, col)) {
                        p0_immediate_threats++;
                    }
                    if (would_create_win(game, PlayerType::Player1, row, col)) {
                        p1_immediate_threats++;
                    }
                }
            }
        }
        
        // Reward having tempo (forcing opponent to respond)
        if (p0_immediate_threats > p1_immediate_threats) {
            tempo_score += 0.5;
        } else if (p1_immediate_threats > p0_immediate_threats) {
            tempo_score -= 0.5;
        }
        
        return tempo_score;
    }

    // Edge avoidance feature - avoid playing on edges early in game
    double edge_avoidance_feature(const Game& game) {
        double edge_penalty = 0.0;
        int total_pieces = 0;
        
        for (int row = 0; row < BOARD_SIZE; row++) {
            for (int col = 0; col < BOARD_SIZE; col++) {
                if (game.is_set(PlayerType::Player0, row, col) || 
                    game.is_set(PlayerType::Player1, row, col)) {
                    total_pieces++;
                }
            }
        }
        
        // Only apply edge penalty in early game
        if (total_pieces < BOARD_SIZE * BOARD_SIZE / 3) {
            for (int row = 0; row < BOARD_SIZE; row++) {
                for (int col = 0; col < BOARD_SIZE; col++) {
                    bool is_edge = (row == 0 || row == BOARD_SIZE - 1 || 
                                   col == 0 || col == BOARD_SIZE - 1);
                    
                    if (is_edge) {
                        if (game.is_set(PlayerType::Player0, row, col)) {
                            edge_penalty -= 0.2;
                        } else if (game.is_set(PlayerType::Player1, row, col)) {
                            edge_penalty += 0.2;
                        }
                    }
                }
            }
        }
        
        return edge_penalty;
    }

    // Trap feature - detect positions that create unavoidable winning sequences
    double trap_feature(const Game& game) {
        double trap_score = 0.0;
        
        for (int row = 0; row < BOARD_SIZE; row++) {
            for (int col = 0; col < BOARD_SIZE; col++) {
                if (!game.is_set(PlayerType::Player0, row, col) && 
                    !game.is_set(PlayerType::Player1, row, col)) {
                    
                    // Check if this position creates a trap (multiple threats that can't all be blocked)
                    if (creates_trap(game, PlayerType::Player0, row, col)) {
                        trap_score += 1.2;
                    }
                    if (creates_trap(game, PlayerType::Player1, row, col)) {
                        trap_score -= 1.2;
                    }
                }
            }
        }
        return trap_score;
    }

    // Check if placing a piece creates a trap (multiple unblockable threats)
    bool creates_trap(const Game& game, PlayerType player, int row, int col) {
        int separate_threats = 0;
        
        // Check for threats in different directions
        int directions[4][2] = {{0,1}, {1,0}, {1,1}, {1,-1}};
        
        for (int d = 0; d < 4; d++) {
            int dr = directions[d][0];
            int dc = directions[d][1];
            
            // Check if this direction would create a threat
            for (int start = -3; start <= 0; start++) {
                int count = 0;
                bool valid_line = true;
                
                for (int i = 0; i < 4; i++) {
                    int nr = row + (start + i) * dr;
                    int nc = col + (start + i) * dc;
                    
                    if (nr < 0 || nr >= BOARD_SIZE || nc < 0 || nc >= BOARD_SIZE) {
                        valid_line = false;
                        break;
                    }
                    
                    if ((nr == row && nc == col) || game.is_set(player, nr, nc)) {
                        count++;
                    } else if (game.is_set(PlayerType(player ^ 1), nr, nc)) {
                        valid_line = false;
                        break;
                    }
                }
                
                if (valid_line && count >= 3) {
                    separate_threats++;
                    break;
                }
            }
        }
        
        return separate_threats >= 2;
    }

    // Mobility feature - reward having more move options
    double mobility_feature(const Game& game) {
        double mobility_score = 0.0;
        
        int p0_options = count_move_options(game, PlayerType::Player0);
        int p1_options = count_move_options(game, PlayerType::Player1);
        
        mobility_score += (p0_options - p1_options) * 0.05;
        
        return mobility_score;
    }

    // Count available move options for a player
    int count_move_options(const Game& game, PlayerType player) {
        int options = 0;
        
        for (int row = 0; row < BOARD_SIZE; row++) {
            for (int col = 0; col < BOARD_SIZE; col++) {
                if (!game.is_set(PlayerType::Player0, row, col) && 
                    !game.is_set(PlayerType::Player1, row, col)) {
                    
                    // Check if this move creates any positive value
                    if (evaluates_move_positively(game, player, row, col)) {
                        options++;
                    }
                }
            }
        }
        return options;
    }

    // Simple heuristic to check if a move has positive value
    bool evaluates_move_positively(const Game& game, PlayerType player, int row, int col) {
        // Check if move creates any line of 2 or more
        int directions[4][2] = {{0,1}, {1,0}, {1,1}, {1,-1}};
        
        for (int d = 0; d < 4; d++) {
            int dr = directions[d][0];
            int dc = directions[d][1];
            
            int count = 1;
            // Check positive direction
            for (int i = 1; i < 4; i++) {
                int nr = row + i * dr;
                int nc = col + i * dc;
                if (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE &&
                    game.is_set(player, nr, nc)) {
                    count++;
                } else {
                    break;
                }
            }
            // Check negative direction
            for (int i = 1; i < 4; i++) {
                int nr = row - i * dr;
                int nc = col - i * dc;
                if (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE &&
                    game.is_set(player, nr, nc)) {
                    count++;
                } else {
                    break;
                }
            }
            
            if (count >= 2) return true;
        }
        return false;
    }

    // Structure feature - reward creating strong positional structures
    double structure_feature(const Game& game) {
        double structure_score = 0.0;
        
        for (int row = 0; row < BOARD_SIZE; row++) {
            for (int col = 0; col < BOARD_SIZE; col++) {
                if (game.is_set(PlayerType::Player0, row, col)) {
                    structure_score += evaluate_piece_structure(game, PlayerType::Player0, row, col);
                } else if (game.is_set(PlayerType::Player1, row, col)) {
                    structure_score -= evaluate_piece_structure(game, PlayerType::Player1, row, col);
                }
            }
        }
        return structure_score;
    }

    // Evaluate the structural value of a piece position
    double evaluate_piece_structure(const Game& game, PlayerType player, int row, int col) {
        double structure_value = 0.0;
        
        // Reward pieces that form strong geometric patterns
        int directions[4][2] = {{0,1}, {1,0}, {1,1}, {1,-1}};
        
        for (int d = 0; d < 4; d++) {
            int dr = directions[d][0];
            int dc = directions[d][1];
            
            // Check for L-shapes and other strong patterns
            int line_length = 1;
            
            // Count in positive direction
            for (int i = 1; i < 3; i++) {
                int nr = row + i * dr;
                int nc = col + i * dc;
                if (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE &&
                    game.is_set(player, nr, nc)) {
                    line_length++;
                } else {
                    break;
                }
            }
            
            // Count in negative direction
            for (int i = 1; i < 3; i++) {
                int nr = row - i * dr;
                int nc = col - i * dc;
                if (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE &&
                    game.is_set(player, nr, nc)) {
                    line_length++;
                } else {
                    break;
                }
            }
            
            // Reward longer lines exponentially
            if (line_length >= 2) {
                structure_value += line_length * 0.1;
            }
        }
        
        return structure_value;
    }

    // Defensive pattern feature - recognize and reward defensive formations
    double defensive_pattern_feature(const Game& game) {
        double defensive_score = 0.0;
        
        for (int row = 0; row < BOARD_SIZE; row++) {
            for (int col = 0; col < BOARD_SIZE; col++) {
                if (game.is_set(PlayerType::Player0, row, col)) {
                    defensive_score += evaluate_defensive_value(game, PlayerType::Player0, row, col);
                } else if (game.is_set(PlayerType::Player1, row, col)) {
                    defensive_score -= evaluate_defensive_value(game, PlayerType::Player1, row, col);
                }
            }
        }
        return defensive_score;
    }

    // Evaluate how well a piece contributes to defense
    double evaluate_defensive_value(const Game& game, PlayerType player, int row, int col) {
        double defensive_value = 0.0;
        PlayerType opponent = PlayerType(player ^ 1);
        
        // Check how many opponent threats this piece blocks or disrupts
        int directions[4][2] = {{0,1}, {1,0}, {1,1}, {1,-1}};
        
        for (int d = 0; d < 4; d++) {
            int dr = directions[d][0];
            int dc = directions[d][1];
            
            // Check lines passing through this position
            for (int start = -3; start <= 0; start++) {
                int opponent_count = 0;
                int empty_count = 0;
                bool line_valid = true;
                
                for (int i = 0; i < 4; i++) {
                    int nr = row + (start + i) * dr;
                    int nc = col + (start + i) * dc;
                    
                    if (nr < 0 || nr >= BOARD_SIZE || nc < 0 || nc >= BOARD_SIZE) {
                        line_valid = false;
                        break;
                    }
                    
                    if (nr == row && nc == col) {
                        // This is our defensive piece
                        continue;
                    } else if (game.is_set(opponent, nr, nc)) {
                        opponent_count++;
                    } else if (!game.is_set(player, nr, nc)) {
                        empty_count++;
                    } else {
                        // Friendly piece - this line is not a threat
                        line_valid = false;
                        break;
                    }
                }
                
                // If opponent had 2-3 pieces in this line, our piece provides good defense
                if (line_valid && opponent_count >= 2) {
                    defensive_value += 0.3 * opponent_count;
                }
            }
        }
        
        return defensive_value;
    }

    // Endgame feature - adjust strategy based on game phase
    double endgame_feature(const Game& game) {
        double endgame_score = 0.0;
        
        int total_pieces = 0;
        int p0_pieces = 0;
        int p1_pieces = 0;
        
        for (int row = 0; row < BOARD_SIZE; row++) {
            for (int col = 0; col < BOARD_SIZE; col++) {
                if (game.is_set(PlayerType::Player0, row, col)) {
                    p0_pieces++;
                    total_pieces++;
                } else if (game.is_set(PlayerType::Player1, row, col)) {
                    p1_pieces++;
                    total_pieces++;
                }
            }
        }
        
        // Determine game phase
        double game_progress = (double)total_pieces / (BOARD_SIZE * BOARD_SIZE);
        
        if (game_progress > 0.7) {
            // Endgame: prioritize forcing moves and piece activity
            int p0_threats = 0;
            int p1_threats = 0;
            
            for (int row = 0; row < BOARD_SIZE; row++) {
                for (int col = 0; col < BOARD_SIZE; col++) {
                    if (!game.is_set(PlayerType::Player0, row, col) && 
                        !game.is_set(PlayerType::Player1, row, col)) {
                        
                        if (would_create_win(game, PlayerType::Player0, row, col)) {
                            p0_threats++;
                        }
                        if (would_create_win(game, PlayerType::Player1, row, col)) {
                            p1_threats++;
                        }
                    }
                }
            }
            
            endgame_score += (p0_threats - p1_threats) * 0.4;
        }
        
        return endgame_score;
    }
};
#endif // CONNECT4_HF_HPP
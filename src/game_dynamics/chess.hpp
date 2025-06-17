#include <vector>

class Chess{
public:
    // row major order
    using BitBoardT = uint64_t;
    using MoveT = std::array<int, 3>; // piece type, from, to
    enum PlayerT{
        WHITE = 0,
        BLACK = 1
    };
    enum PieceTypeT{
        PAWN = 0,
        KNIGHT = 1,
        BISHOP = 2,
        ROOK = 3,
        QUEEN = 4,
        KING = 5
    };
    constexpr int NUM_ROWS = 8;
    constexpr int NUM_COLS = 8;
    // Two colors, 6 pieces
    BitBoardT boards[2][6];
    std::vector<MoveT> moves;
    PlayerT cur_player = WHITE;

    __attribute__((always_inline)) bool inline is_occupied(int square, PlayerT player, PieceTypeT piece){
        return boards[player][piece] & (1ULL << square);
    }

    __attribute__((always_inline)) bool inline other_player(PlayerT player){
        return PlayerT(player ^ 1);
    }

    __attribute__((always_inline)) bool inline reflect_board(BitBoardT board){
        BitBoardT row_mask = 0xFF;
        BitBoardT new_board = 0;
        #pragma unroll
        for (int row = 0; row < NUM_ROWS; row++) {
            new_board |= ((board >> (NUM_COLS * row)) & row_mask) << (NUM_COLS * (7 - row));
        }
        return new_board;
    }

    __attribute__((always_inline)) bool inline reflect_board(BitBoardT board){
        BitBoardT row_mask = 0xFF;
        BitBoardT new_board = 0;
        #pragma unroll
        for (int row = 0; row < NUM_ROWS; row++) {
            new_board |= ((board >> (NUM_COLS * row)) & row_mask) << (NUM_COLS * (7 - row));
        }
        return new_board;
    }

    void reset_moves(){
        moves.clear();
    }

    void add_pawn_move(){
        // board stored in PoV of player
        BitBoardT pawn_pos = boards[cur_player][PAWN];
        while(pawn_pos){
            int from = __builtin_ctzll(pawn_pos);
            pawn_pos &= pawn_pos - 1;
            int row = from / 8, col = from % 8;
            // Promotions

            assert(row < 7); // promotions - pawn cannot be on 8 th
            // Advances
            int to = from + 8;
            if(!is_occupied(to, cur_player, PAWN)){
                moves.push_back({PAWN, from, to});
            }
            // Captures
            int to = from + 7;
            if (col > 0 && is_occupied(to, other_player(cur_player))) 
            // En passant??
        }

    }
    void add_rook_move(){
        rook_pos = boards[player][ROOK];
        while(rook_pos){
            int from = __builtin_ctzll(rook_pos);
            
        }
    }
};
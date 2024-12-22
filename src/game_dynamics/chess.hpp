#include <vector>

class Chess{
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
public:
    // Two colors, 6 pieces
    BitBoardT boards[2][6];
    std::vector<MoveT> moves;
    PlayerT player = WHITE;

    __attribute__((always_inline)) bool inline is_occupied(int square, PieceTypeT piece){
        return boards[player][piece] & (1ULL << square);
    }


    void reset_moves(){
        moves.clear();
    }

    void add_pawn_move(){
        pawn_pos = boards[player][PAWN];
        while(pawn_pos){
            int from = __builtin_ctzll(pawn_pos);
            pawn_pos &= pawn_pos - 1;
            // Add pawn moves
            int to = from + 8;
            if(to < 64 && !is_occupied(to, PAWN)){
                moves.push_back({PAWN, from, to});
            }
            // Captures
        }
    }
    void add_rook_move(){
        rook_pos = boards[player][ROOK];
        while(rook_pos){
            int from = __builtin_ctzll(rook_pos);
            
        }
    }
};
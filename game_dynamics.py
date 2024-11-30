import numpy as np

class Game():
    
    def __init__(self):
        raise NotImplementedError
    
    def is_terminal(self) -> bool:
        raise NotImplementedError
    
    def get_result(self):
        raise NotImplementedError
    
    def get_player(self):
        raise NotImplementedError
    
    def get_valid_moves(self):
        raise NotImplementedError
    
    def make_move(self, move):
        raise NotImplementedError
    
    def clone(self):
        raise NotImplementedError
    
class TicTacToe(Game):
    
    num_players = 2
    def_result = np.zeros(num_players)
    def __init__(self):
        self.board = np.zeros((3, 3))
        self.player = 1
        self.result = np.zeros(TicTacToe.num_players)
        
    def clone(self):
        new_game = TicTacToe()
        new_game.board = np.copy(self.board)
        new_game.player = self.player
        new_game.result = np.copy(self.result)
        return new_game
    
    def is_def_result(self):
        return np.all(self.result == TicTacToe.def_result)
        
    def is_terminal(self):
        self.result = self.get_result()
        return not self.is_def_result() or np.all(self.board != 0)
    
    def get_valid_moves(self):
        return np.stack(np.where(self.board == 0), axis=-1)
    
    def make_move(self, move):
        move = tuple(move)
        self.board[move] = self.player
        self.player = -self.player
        
    def get_result(self):
        if not self.is_def_result():
            return self.result
        if np.any(np.sum(self.board, axis = 0) == 3) or\
            np.any(np.sum(self.board, axis = 1) == 3) or\
            np.trace(self.board) == 3 or\
            np.trace(np.fliplr(self.board)) == 3: 
            self.result = np.array([1, -1])
        if np.any(np.sum(self.board, axis = 0) == -3) or\
            np.any(np.sum(self.board, axis = 1) == -3) or\
            np.trace(self.board) == -3 or\
            np.trace(np.fliplr(self.board)) == -3: 
            self.result = np.array([-1, 1])
        return self.result
    
    def get_player(self):
        return 0 if self.player == 1 else 1
    
    def __str__(self):
        return str(self.board)
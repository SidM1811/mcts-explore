import numpy as np
from game_dynamics import Game, TicTacToe

class MCTS():
    
    '''Player 1 is 0, Player 2 is 1'''
    
    def __init__(self, game_state, parent = None, cur_idx = None):
        self.game_state: Game = game_state
        self.valid_moves = game_state.get_valid_moves()
        
        # Need for backpropagation
        self.cur_idx = cur_idx
        self.parent = parent
        
        self.children = [None] * len(self.valid_moves)
        self.visit = np.zeros(len(self.valid_moves))
        self.Q = np.zeros(len(self.valid_moves))
        
        self.exp_param = 1.0
        self.total_visit = 0
        self.num_sim = 16
        
    def is_terminal(self):
        return self.game_state.is_terminal()
        
    def selection_step(self):
        empty_idx = np.where(self.visit == 0)[0]
        if len(empty_idx) > 0:
            return np.random.choice(empty_idx)
        best_idx = np.argmax(self.Q + self.exp_param * np.sqrt(np.log(self.total_visit) / self.visit))
        return best_idx
    
    def expansion(self, move_idx):
        assert self.children[move_idx] is None
        new_state = self.game_state.clone()
        new_state.make_move(self.valid_moves[move_idx])
        self.children[move_idx] = MCTS(new_state)
        
        self.children[move_idx].cur_idx = move_idx
        self.children[move_idx].parent = self
        
        result = self.children[move_idx].simulation()
        return result, self.children[move_idx]
    
    def simulation(self):
        value = np.float64(0)
        for rollouts in range(self.num_sim):
            new_state = self.game_state.clone()
            while not new_state.is_terminal():
                valid_moves = new_state.get_valid_moves()
                move = valid_moves[np.random.choice(len(valid_moves))]
                new_state.make_move(move)
            value += new_state.get_result() # vector of results
        update = value / self.num_sim
        return update
    
    def backpropagate_step(self, update, move_idx):
        node_player = self.game_state.get_player()
        self.visit[move_idx] += 1
        self.Q[move_idx] += (update[node_player] - self.Q[move_idx]) / self.visit[move_idx]
        self.total_visit += 1
        
    def backpropagate(self, update):
        if self.parent is not None:
            assert self.cur_idx is not None
            self.parent.backpropagate_step(update, self.cur_idx)
            self.parent.backpropagate(update)
            
    def search(self) -> None:
        if self.is_terminal():
            update = self.game_state.get_result()
            self.backpropagate(update)
            return
        sel_idx = self.selection_step()
        if self.children[sel_idx] is None:
            update, child = self.expansion(sel_idx)
            child.backpropagate(update)
        else:
            self.children[sel_idx].search()
            
    def __str__(self, level=0):
        ret = "\t" * level + repr(self) + "\n"
        for child in self.children:
            if child is not None:
                ret += child.__str__(level + 1)
        return ret

    def __repr__(self):
        return f"MCTS(Node: {self.cur_idx}, Q: {self.Q}, Visits: {self.visit})"

if __name__ == "__main__":
    game_state = TicTacToe()
    root_node = MCTS(game_state)
    try:
        for ply in range(9):
            for it in range(100):
                root_node.search()
            move_idx = np.argmax(root_node.Q)
            root_node = root_node.children[move_idx]
            print(root_node.game_state.board)
            print()
    except:
        pass
            
#!/usr/bin/python
from tronverse.Program import Program
from tronverse.GameGrid import GameGrid

RED = (255, 0, 0)
BLUE = (0, 0, 255)
BLACK = (0, 0, 0)

UP = 0
RIGHT = 1
DOWN = 2
LEFT = 3

def startGame():
    
    #
    # Build the game grid.
    #
    
    gameGrid = GameGrid(300, 300)
    gameGrid.SetFrameRate(20)
    
    #
    # Create the programs (players).
    #
    
    clu = Program(0)
    clu.UseBaton(DOWN, 10, 10)
    
    ram = Program(1)
    ram.UseBaton(UP, gameGrid.Width - 10, gameGrid.Height - 10)
    
    #
    # Load programs in to the game grid.
    #
    
    gameGrid.LoadProgram(clu)
    gameGrid.LoadProgram(ram)
    
    #
    # Engine.
    #
    
    while gameGrid.IsRunning():
        gameGrid.DoWork()

if __name__ == "__main__":
    startGame()

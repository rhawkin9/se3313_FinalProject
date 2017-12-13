from .LightCycle import LightCycle

RED = (255, 0, 0)
BLUE = (0, 0, 255)
BLACK = (0, 0, 0)

UP = 0
RIGHT = 1
DOWN = 2
LEFT = 3

class Program(object):
    '''
    A user program
    '''
    
    def __init__(self, Id):
        '''
        Initializes a new instance of the Program class.
        '''
        
        self._Health = 1
        self._Id = Id
        self.LightCycle = None
    
    def Derezz(self):
        '''
        Returns a value indicating whether the program should derezz.
        '''
        
        # Decrease health by one.
        self._Health = self._Health - 1
        
        # Return boolean indicating whether to derezz the player.
        return self._Health == 0
    
    def Name(self):
        '''
        Returns the programs name.
        '''
        
        return "Player " + self._Id
    
    def UseBaton(self, Direction, StartX, StartY):
        
        
        '''
        Use the baton to generate the light cycle.
        '''

        color = RED

        if self._Id == 0:
            color = RED
        elif self._Id == 1:
            color = BLUE


        self.LightCycle = LightCycle(color, Direction, StartX, StartY)
RED = (255, 0, 0)
BLUE = (0, 0, 255)
BLACK = (0, 0, 0)

UP = 0
RIGHT = 1
DOWN = 2
LEFT = 3

class LightCycle(object):
    '''
    Light cycle vehicle
    '''

    def __init__(self, Color, Direction = UP, StartX = 0, StartY = 0):
        '''
        Initializes a new instance of the LightCycle class.
        '''
        
        self.RibbonColor = Color
        self.Direction = Direction
        self._LightRibbon = [(StartX, StartY)]
        self._Speed = 4
    
    def ChangeDirection(self, NewHeading):
        '''
        Change light cycle direction.
        '''
        
        # Check to make sure the bike is not pulling a 180 degree turn.
        if ((self.Direction == UP and NewHeading ==  DOWN)
            or (self.Direction == DOWN and NewHeading == UP)
            or (self.Direction == RIGHT and NewHeading == LEFT)
            or (self.Direction == LEFT and NewHeading == RIGHT)):
            pass
        else:
            self.Direction = NewHeading
    
    def CollisionWithLightRibbon(self, Ribbon):
        '''
        Check each point of a ribbon if we have a collision.
        '''
        
        for coord in Ribbon:
            if self._LightRibbon.count(coord) > 0:
                return True
        
        return False
    
    def CollisionWithWall(self, PosX, PosY):
        '''
        Check all possible wall collision combinations.
        '''
        
        # First find the current position.
        currentPosition = self._LightRibbon[-1]
        
        return ((currentPosition[0] < 1 or currentPosition[0] >= PosX)
                or (currentPosition[1] < 1 or currentPosition[1] >= PosY)
                or (self._LightRibbon.count(currentPosition) > 1))
    
    def LightRibbon(self):
        '''
        Return all vertices of the light ribbon.
        '''
        
        return self._LightRibbon[:]
    
    def Move(self):
        '''
        Move the light cycle one unit in its current direction.
        '''
        
        # First find the current position.
        currentPosition = self._LightRibbon[-1]
        
        # Determine which direction to move.
        if self.Direction == UP:
            self._LightRibbon.append((currentPosition[0], currentPosition[1] - self._Speed))
        elif self.Direction == RIGHT:
            self._LightRibbon.append((currentPosition[0] + self._Speed, currentPosition[1]))
        elif self.Direction == DOWN:
            self._LightRibbon.append((currentPosition[0], currentPosition[1] + self._Speed))
        else:
            self._LightRibbon.append((currentPosition[0] - self._Speed, currentPosition[1]))
        
        # Return the light ribbon.
        return self._LightRibbon
    
    def SetSpeed(self, NewSpeed):
        self._Speed = NewSpeed
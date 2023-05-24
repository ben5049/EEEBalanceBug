
class Node:
    # state is either 1 or 2 - 1 being visited once, 2 being visited twice
    # 3 is special state - signifies its the exit of the maze
    # when visited twice, it is treated as a "dead end" and will not be 
    # re-traversed, but will be added to children of prior node
    def __init__(self, x,y,startend = 0):
        self.position = (x,y)
        self.state = 1
        self.is_startend=startend
        self.edges = {"N":None,"E":None,"S":None,"W":None}

    def visit(self):
        self.state = 2

    def setend(self):
        self.state = 3
    
    def connect(self,n1,direction:str):
        a_dir = [direction for direction in self.edges if self.edges[direction] == None]
        if(direction not in a_dir):
            return "Direction already occupied"
        else:
            match(direction.upper()):
                case "N":
                    if n1.position[0] == self.position[0] and 0<=n1.position[0]<=360 and 0<=n1.position[1]<=240:
                        self.edges["N"] = n1
                        n1.edges["S"] = self
                        return 1
                    else:
                        return "new node not aligned"
                case "E":
                    if n1.position[1] == self.position[1] and 0<=n1.position[0]<=360 and 0<=n1.position[1]<=240:
                        n1.edges["W"] = self
                        self.edges["E"] = n1
                        return 1
                    else:
                        return "new node not aligned"
                case "S":
                    if n1.position[0] == self.position[0] and 0<=n1.position[0]<=360 and 0<=n1.position[1]<=240:
                        n1.edges["N"] = self
                        self.edges["S"] = n1
                        return 1
                    else:
                        return "new node not aligned"
                case "W":
                    if n1.position[1] == self.position[1] and 0<=n1.position[0]<=360 and 0<=n1.position[1]<=240:
                        n1.edges["E"] = self
                        self.edges["W"] = n1
                        return 1
                    else:
                        return "new node not aligned"

    def __str__(self):
        return str(self.position)
    
def valid_connection(n1:Node,direction,length):
    match(direction.upper()):
            case "N":
                if 0<=n1.position[0]+length<=360:
                    return 1
                else:
                    return 0
            case "E":
                if 0<=n1.position[1]+length<=240:
                    return 1
                else:
                    return 0
            case "S":
                if 0<=n1.position[0]-length<=360:
                    return 1
                else:
                    return 0
            case "W":
                if 0<=n1.position[1]-length<=240:
                    return 1
                else:
                    return 0
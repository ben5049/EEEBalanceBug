# Aranya Gupta
# 19/5/2023
# TODO: finish tremaux algorithm (mainly helper funcs), test everything
class Node:
    # state is either 1 or 2 - 1 being visited once, 2 being visited twice
    # 3 is special state - signifies its the exit of the maze
    # when visited twice, it is treated as a "dead end" and will not be 
    # re-traversed, but will be added to children of prior node
    state = 0 
    position = 0
    def __init__(self, position):
        self.position = position
        self.state = 1

    def visit(self):
        self.state = 2

    def setend(self):
        self.state = 3

    def __str__(self):
        return str(self.position)

# abstract funcs - to be implemented later. will use recursion with tremaux()
# so that it is a depth-first search. dependent on how tremaux alg will control 
# rover

def dataRequest():
    return

def clockwiseTurn90():
    return

def anticlockwiseTurn90():
    return

def step_forward():
    return

def go_back(orientation, priornode, position):
    # if facing right
    xdiff = position[0]-priornode.position[0]
    ydiff = position[1]-priornode.position[1]
    if orientation==0:
        clockwiseTurn90()
        clockwiseTurn90()
        step_forward()
        if ydiff > 0: # 
            clockwiseTurn90()
            step_forward()
            clockwiseTurn90()
            clockwiseTurn90()
        elif ydiff < 0:
            anticlockwiseTurn90()
            step_forward()
            clockwiseTurn90()
            clockwiseTurn90()
        else:
            step_forward()
            clockwiseTurn90()
            clockwiseTurn90()

    # if facing left
    elif orientation==1:
        clockwiseTurn90()
        clockwiseTurn90()
        step_forward()
        if ydiff > 0: # 
            anticlockwiseTurn90()
            step_forward()
            clockwiseTurn90()
            clockwiseTurn90()
        elif ydiff < 0:
            clockwiseTurn90()
            step_forward()
            clockwiseTurn90()
            clockwiseTurn90()
        else:
            step_forward()
            clockwiseTurn90()
            clockwiseTurn90()

    # if facing up
    elif orientation==2:
        clockwiseTurn90()
        clockwiseTurn90()
        step_forward()
        if xdiff > 0: # 
            clockwiseTurn90()
            step_forward()
            clockwiseTurn90()
            clockwiseTurn90()
        elif ydiff < 0:
            anticlockwiseTurn90()
            step_forward()
            clockwiseTurn90()
            clockwiseTurn90()
        else:
            step_forward()
            clockwiseTurn90()
            clockwiseTurn90()

    # if facing down
    elif orientation == 3:
        clockwiseTurn90()
        clockwiseTurn90()
        step_forward()
        if xdiff > 0: # 
            anticlockwiseTurn90()
            step_forward()
            clockwiseTurn90()
            clockwiseTurn90()
        elif ydiff < 0:
            clockwiseTurn90()
            step_forward()
            clockwiseTurn90()
            clockwiseTurn90()
        else:
            step_forward()
            clockwiseTurn90()
            clockwiseTurn90()
    return

def visit_left(priornode):
    # turn left    
    step_forward()
    anticlockwiseTurn90()
    step_forward()
    
    # wait for new position, whereat, orientation
    p, w, o = dataRequest()

    # tremaux(position, whereat, priornode) to search left part of maze
    priorwhereat = 1
    tremaux(p, w, o, priornode)
    

def visit_right(priornode):
    # turn right
    step_forward()
    clockwiseTurn90()
    step_forward()
    # wait for new position, whereat, 
    p, w, o = dataRequest()
    # tremaux(position, whereat, priornode) to search right part of maze
    priorwhereat = 1
    tremaux(p, w, o, priornode)

def visit_straight(priornode):
    # go straight
    step_forward()
    step_forward()
    # wait for new position, whereat, 
    p, w, o = dataRequest()
    # tremaux(position, whereat, priornode) to search straight part of maze
    priorwhereat = 1
    tremaux(p, w, o, priornode)


# position is x, y from starting position ie dead reckoning
# whereat is passage - 0, junc - 1, deadend - 2, exit - 3
# whereat may be done on this side rather than on fpga 
# orientation is 0 - right, 1 - left, 2 - up, 3 - down
# currently, after each movement, relevant data is sent to program
# which then uses tremaux to find the next step the rover should take

# tree represented as hash table with each node having its children as an array of values
nodes = {}
priorwhereat = 0
def tremaux(position, whereat, orientation, priornode):
    if whereat == 0: 
        if priorwhereat == 0: 
            return step_forward()
        elif priorwhereat == 1:
            if position not in [node.position for node in nodes]: #if we haven't ever visited this node ...
                n = Node(position)
                for node in nodes:
                    if node.position == priornode.position:
                        nodes[node].append(n)
                        break
                nodes[n] = []
                return step_forward()
            else: #if we have previously visited this node .. 
                for node in nodes:
                    if node.position == position: 
                        node.visit()
                        return go_back(orientation, priornode, position)
    elif whereat == 1:
        for node in nodes:
            if node.position == position:  #if we've already visited this node mark it as dead and go back
                node.visit()
                return go_back(orientation, priornode, position)
            
        # if we haven't visited this node, check all possible other routes, then backtrack
        n = Node(position)
        nodes[n] = []
        visit_left(n)
        go_back(orientation, n, position)
        visit_right(n)
        go_back(orientation, n, position)
        visit_straight(n)
        go_back(orientation, n, position)
        n.visit()
        go_back(orientation, n, position)

    elif whereat == 2:
        n = Node(position)
        nodes[n] = []
        n.visit()
        go_back(orientation, priornode, position)

    elif whereat == 3:
        n = Node(position)
        nodes[n] = []
        n.setend()
        go_back(orientation, priornode, position)
        

    priorwhereat = whereat
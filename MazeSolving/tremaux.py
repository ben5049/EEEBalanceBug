# Aranya Gupta
# 19/5/2023
# TODO: modify movement so it can handle non-right angle turning
from random import randint
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

def dataRequest(V):
    # making fake position data for testing
    position = (randint(0,500),randint(0,500))
    whereat = randint(0,3)
    orientation = 0
    V.visualiser(position, whereat, orientation)
    input()
    return position, whereat, orientation

def clockwiseTurn90():
    return

def anticlockwiseTurn90():
    return

def step_forward():
    return

def go_back(orientation, priornode, position, priorwhereat, V):
    # if facing right
    xdiff = position[0]-priornode.position[0]
    ydiff = position[1]-priornode.position[1]
    if priorwhereat == 0:
        if orientation==0:
            while xdiff > 0:
                clockwiseTurn90()
                clockwiseTurn90()
                step_forward()
                position = dataRequest(V)[0]
                xdiff = position[0]-priornode.position[0]
        elif orientation == 1:
            while xdiff < 0:
                clockwiseTurn90()
                clockwiseTurn90()
                step_forward()
                position = dataRequest(V)[0]
                xdiff = position[0]-priornode.position[0]
        elif priorwhereat == 2:
            while ydiff < 0:
                clockwiseTurn90()
                clockwiseTurn90()
                step_forward()
                position = dataRequest(V)[0]
                ydiff = position[1]-priornode.position[1]
        elif priorwhereat == 3:
            while ydiff > 0:
                clockwiseTurn90()
                clockwiseTurn90()
                step_forward()
                position = dataRequest(V)[0]
                ydiff = position[1]-priornode.position[1]
    elif priorwhereat == 1:
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

def visit_left(priornode, priorwhereat, V, nodes):
    # turn left    
    step_forward()
    anticlockwiseTurn90()
    step_forward()
    
    # wait for new position, whereat, orientation
    p, w, o = dataRequest(V)

    # tremaux(position, whereat, priornode) to search left part of maze
    tremaux(p, w, o, priornode, priorwhereat, V, nodes)
    

def visit_right(priornode, priorwhereat, V, nodes):
    # turn right
    step_forward()
    clockwiseTurn90()
    step_forward()
    # wait for new position, whereat, 
    p, w, o = dataRequest(V)
    # tremaux(position, whereat, priornode) to search right part of maze
    priorwhereat = 1
    tremaux(p, w, o, priornode, priorwhereat, V, nodes)

def visit_straight(priornode, priorwhereat, V, nodes):
    # go straight
    step_forward()
    step_forward()
    # wait for new position, whereat, 
    p, w, o = dataRequest(V)
    # tremaux(position, whereat, priornode) to search straight part of maze
    priorwhereat = 1
    tremaux(p, w, o, priornode, priorwhereat, V, nodes)


# position is x, y from starting position ie dead reckoning
# whereat is passage - 0, junc - 1, deadend - 2, exit - 3
# whereat may be done on this side rather than on fpga 
# orientation is 0 - right, 1 - left, 2 - up, 3 - down
# currently, after each movement, relevant data is sent to program
# which then uses tremaux to find the next step the rover should take

# tree represented as hash table with each node having its children as an array of values

def tremaux(position, whereat, orientation, priornode, priorwhereat, V, nodes):
    print(nodes)
    if whereat == 0: 
        if priorwhereat == 0: 
            step_forward()
            p, w, o = dataRequest(V)
            tremaux(p, w, o, priornode, whereat, V, nodes)
        elif priorwhereat == 1:
            if position not in [node.position for node in nodes]: #if we haven't ever visited this node ...
                n = Node(position)
                for node in nodes:
                    if node.position == priornode.position:
                        nodes[node].append(n)
                        break
                nodes[n] = []
                step_forward()
                p, w, o = dataRequest(V)
                tremaux(p, w, o, priornode, whereat, V, nodes)
            else: #if we have previously visited this node .. 
                for node in nodes:
                    if node.position == position: 
                        node.visit()
                        go_back(orientation, priornode, position, priorwhereat, V)
                        break
    elif whereat == 1:
        flag = True
        for node in nodes:
            if node.position == position:  #if we've already visited this node mark it as dead and go back
                node.visit()
                go_back(orientation, priornode, position, priorwhereat, V)
                flag = False
                break
        # if we haven't visited this node, check all possible other routes, then backtrack
        if flag:
            n = Node(position)
            nodes[n] = []
            visit_left(n, whereat, V, nodes)
            visit_right(n, whereat, V, nodes)
            visit_straight(n, whereat, V, nodes)
            n.visit()
            go_back(orientation, n, position, priorwhereat, V)

    elif whereat == 2:
        n = Node(position)
        nodes[n] = []
        n.visit()
        go_back(orientation, priornode, position, priorwhereat, V)

    elif whereat == 3:
        n = Node(position)
        nodes[n] = []
        n.setend()
        go_back(orientation, priornode, position, priorwhereat, V)
    
    # once we get back to the start, the maze has finished being mapped out
    if position == (0,0):
        return nodes 
        
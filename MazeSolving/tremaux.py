# Aranya Gupta
# 19/5/2023
from random import randint
from math import atan, degrees
from triangulate import triangulate
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

    def updatePos(self, x, y):
        self.position = (x, y)

    def __str__(self):
        return str(self.position)

# abstract funcs - to be implemented later. will use recursion with tremaux()
# so that it is a depth-first search. dependent on how tremaux alg will control 
# rover

def dataRequest():
    # receive data from esp
    position = tuple(int(x) for x in input("pos ").split(","))
    whereat = int(input("whereat "))
    orientation = int(input("orientation "))
    # send data to client
    print(position, whereat, orientation)
    return position, whereat, orientation
# get left, right, up, down junction positions

def spin():
    ang1 = input()
    ang2 = input()
    ang3 = input()
    updatedx, updatedy = triangulate(ang1, ang2, ang3)
    junctionAngs = []
    return junctionAngs, updatedx, updatedy # angle setpoints to turn to

# sets angle in degrees from machine north - positive is clockwise, negative is anticlockwise
# e.g. setAngle(90) will rotate it 90 deg clockwise, setAngle(-90) will rotate it 180 degrees anticlockwise
def setAngle(angle):
    return

# steps forward set amount - may change for variable amounts
def step_forward():
    return

def go_back(orientation, priornode, position, priorwhereat):
    xdiff = position[0]-priornode.position[0]
    ydiff = position[1]-priornode.position[1]
    angle = degrees(atan(ydiff/xdiff))
    if priorwhereat == 0:
        while xdiff > 0:
            setAngle(90-angle) # mathematically should be correct - may not be idk
            step_forward()
            position = dataRequest()[0]
            xdiff = position[0]-priornode.position[0]
            ydiff = position[1]-priornode.position[1]
            angle = degrees(atan(ydiff/xdiff))
    elif priorwhereat == 1: # possible bug - may need to step forward more than once
        setAngle(orientation+180)
        step_forward()
        position = dataRequest()[0]
        xdiff = position[0]-priornode.position[0]
        ydiff = position[1]-priornode.position[1]
        angle = degrees(atan(ydiff/xdiff))
        setAngle(90-angle) # mathematically should be correct - may not be idk
        step_forward()
    return

def visit_branch(priornode, priorwhereat, angle, nodes):
    setAngle(angle)
    step_forward()
    
    # wait for new position, whereat, orientation
    p, w, o = dataRequest()

    # tremaux(position, whereat, priornode) to search branch of maze
    tremaux(p, w, o, priornode, priorwhereat, nodes)

def updateRoverPos(pos):
    return pos

# position is x, y from starting position ie dead reckoning
# whereat is passage - 0, junc - 1, deadend - 2, exit - 3
# whereat may be done on this side rather than on fpga 
# orientation is int showing current angle, 0 is north-pointing
# currently, after each movement, relevant data is sent to program
# which then uses tremaux to find the next step the rover should take

# tree represented as hash table with each node having its children as an array of values

def tremaux(position, whereat, orientation, priornode, priorwhereat, nodes):
    if whereat == 0: 
        if priorwhereat == 0: 
            step_forward()
            p, w, o = dataRequest()
            tremaux(p, w, o, priornode, whereat, nodes)
        elif priorwhereat == 1:
            if position not in [node.position for node in nodes]: #if we haven't ever visited this node ...
                n = Node(position)
                for node in nodes:
                    if node.position == priornode.position:
                        nodes[node].append(n)
                        break
                nodes[n] = []
                step_forward()
                p, w, o = dataRequest()
                tremaux(p, w, o, priornode, whereat, nodes)
            else: #if we have previously visited this node .. 
                for node in nodes:
                    if node.position == position: 
                        node.visit()
                        go_back(orientation, priornode, position, priorwhereat)
                        break
    elif whereat == 1:
        flag = True
        for node in nodes:
            if node.position == position:  #if we've already visited this node mark it as dead and go back
                node.visit()
                go_back(orientation, priornode, position, priorwhereat)
                flag = False
                break
        # if we haven't visited this node, check all possible other routes, then backtrack
        if flag:
            n = Node(position)
            step_forward()
            junctions, newx, newy = spin()
            updateRoverPos((newx, newy))
            n.updatePos(newx, newy)
            nodes[n] = []
            # visit left, right, straight branches
            for ang in junctions:
                visit_branch(n, whereat, ang, nodes)
            n.visit()
            go_back(orientation, n, position, priorwhereat)

    elif whereat == 2:
        n = Node(position)
        nodes[n] = []
        n.visit()
        go_back(orientation, priornode, position, priorwhereat)

    elif whereat == 3:
        n = Node(position)
        nodes[n] = []
        n.setend()
        go_back(orientation, priornode, position, priorwhereat)
    
    # once we get back to the start, the maze has finished being mapped out
    if position == (0,0):
        return nodes 
        
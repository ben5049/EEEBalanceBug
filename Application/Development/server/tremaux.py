# Aranya Gupta
# 31/5/2023
from triangulate import triangulate
from math import abs 
# TODO: general testing & work on threshold checking
THRESHOLD = 2
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

    def thresholding(pos1, pos2):
        if abs(pos1[0] -pos2[0]) < THRESHOLD and abs(pos1[1] -pos2[1]) < THRESHOLD:
            return True
        else:
            return False
            


class Rover():
    DEFAULT_DIST = 100
    name = ""
    # next actions to take - 0: idle, 1: step forward, 2: spin, 3: visit branch, 4: go back, 5: update position, 6: set angle, 7: ignore until reached position
    actions = []
    # tree containing nodes and their edges
    tree = {}

    toreturn = []
    nickname = ""
    priornode = 0
    priorwhereat = 0
    sessionId = -1
    lastSeen = 0
    estop = False 

    pause = False

    def __init__(self, position, whereat, name):
        self.name = name
        n = Node(position)
        self.tree[n] = []
        self.priornode = n
        
        if whereat == 0:
            self.actions.append(1)
            self.priorwhereat = 1
        elif whereat == 1:
            self.actions.append(2)
        self.toreturn = [-1]
        # return 200 ok response 
    
    def __str__(self):
        return str(self.name)
    
    # these all update toreturn, which gives the actual things the rover will do
    
    def step_forward(self):
        self.toreturn.append("step")
    
    def spin(self):
        self.toreturn.append("spin")
    
    def setAngle(self, angle):
        self.toreturn.append("angle " + str(angle))
    
    def go_back(self, node, orientation):
        self.setAngle(orientation+180)
        self.step_forward()
        self.actions = [[7, node]] + self.actions
        
    
    def idle(self):
        self.toreturn.append("idle")
    
    def updatePos(self, newx, newy):
        self.toreturn.append("update position " + str(newx) + " " + str(newy))


    def tremaux(self, position, whereat, potentialbranches, beaconangles, orientation):
        if self.pause:
            return ["idle"]
        if self.toreturn[0] == -1:
            self.step_forward()
            self.toreturn.pop(0)
            return self.toreturn
        else:
            self.toreturn = []
        if len(self.actions)!=0:
            currentAction = self.actions.pop(0)
            if currentAction == 1:
                if whereat == 0:
                    if position not in [nodes.position for nodes in self.tree]:
                        n = Node(position)
                        self.tree[n] = []
                        self.tree[self.priornode].append(n)
                        self.priornode = n
                        self.priorwhereat = 0
                        self.actions = [1] + self.actions
                        if self.priorwhereat == 1:
                            self.step_forward()
                    else:
                        for node in self.tree:
                            if node.position == position:
                                node.visit()
                        self.actions = [[4, self.priornode]] + self.actions
                elif whereat == 1:
                    if position not in [nodes.position for nodes in self.tree]:
                        self.actions = [2] + self.actions
                        self.idle()
                    else:
                        for node in self.tree:
                            if node.position == position:
                                node.visit()
                        self.actions= [[4,self.priornode]] + self.actions
                elif whereat == 2:
                    self.actions = [[4, self.priornode]] + self.actions
                    self.idle()
                elif whereat == 3:
                    n = Node(position)
                    self.tree[n] = []
                    n.setend()
                    self.tree[self.priornode].append(n)
                    self.priornode = n
                    self.actions = [[4, self.priornode]] + self.actions
            elif currentAction == 2:
                self.actions = [3] + self.actions
                self.spin()
            elif currentAction == 3:
                if position in [nodes.position for nodes in self.tree]:
                    for node in self.tree:
                        if node.position == position:
                            node.visit()
                    self.actions = [[4, self.priornode]] + self.actions
                else:
                    newx, newy = triangulate(beaconangles[0], beaconangles[1], beaconangles[2])
                    self.updatePos(newx, newy)
                    n = Node(newx, newy)
                    self.tree[n] = []
                    self.tree[self.priornode].append(n)

                    self.actions = [[4, self.priornode]] + self.actions
                    self.priornode = n
                    self.priorwhereat = 1
                    for i in potentialbranches:
                        self.actions = [[3, i], [4, self.priornode]] + self.actions 
                               
            elif currentAction[0] == 3:
                self.actions = [[6,currentAction[1]], 1] + self.actions
            elif currentAction[0] == 4:
                currentAction[1].visit()
                self.go_back(currentAction[1], orientation)
            elif currentAction[0] == 6:
                self.setAngle(currentAction[1], position)
                self.step_forward()
            elif currentAction[0] == 7:
                if position != currentAction[1].position:
                    self.actions = [[7, currentAction[1]]] + self.actions
        else:
            self.toreturn.append("DONE")

        return self.toreturn

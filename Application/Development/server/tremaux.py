# Aranya Gupta
# Started 18/5/2023, major revision 31/5/2023
from triangulate import triangulate
from time import time
THRESHOLD = 2
# Node class storing position data and whether or not a node has been visited
class Node:
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
    startup = 0
    pause = False

    def __init__(self, position, whereat, name):
        self.name = name
        n = Node(position)
        self.tree[n] = []
        self.priornode = n
        self.startup = int(time())
        if whereat == 0:
            self.actions.append(1)
            self.priorwhereat = 1
        elif whereat == 1:
            self.actions.append(2)
        self.toreturn = [-1]
        # return 200 ok response 
    
    def __str__(self):
        return str(self.name)
    
    def thresholding(self, pos1, pos2):
        if abs(pos1[0] -pos2[0]) < THRESHOLD and abs(pos1[1] -pos2[1]) < THRESHOLD:
            return True
        else:
            return False
    
    # these all update toreturn, which gives the actual things the rover will do
    # step - 0, spin - 1, angle - 2, idle - 3, update position - 4, done - 5
    def step_forward(self):
        self.toreturn.append(0) 
    
    def spin(self):
        self.toreturn.append(1)
    
    def setAngle(self, angle):
        self.toreturn.append(2)
        self.toreturn.append(float(angle))
    
    def go_back(self, node, orientation):
        self.setAngle(orientation+180)
        self.step_forward()
        self.actions = [[7, node]] + self.actions
        
    
    def idle(self):
        self.toreturn.append(3)
    
    def updatePos(self, newx, newy):
        self.toreturn.append(4)
        self.toreturn.append(float(newx))
        self.toreturn.append(float(newy))


    def tremaux(self, position, whereat, potentialbranches, beaconangles, orientation):
        # if paused, remain idle
        if self.pause:
            return [3]
        
        # special state when rover has just started - tell it to move forward
        if self.toreturn[0] == -1:
            self.step_forward()
            self.toreturn.pop(0)
            return self.toreturn
        else:
            self.toreturn = []
        
        # State machine logic for what to do while the rover is running
        if len(self.actions)!=0:
            # action is misnomer - really correspons to state. self.toretun stores actual output actions 
            currentAction = self.actions.pop(0)
            # check if in state 1
            if currentAction == 1:
                # check if at a passage
                if whereat == 0:
                    # check if position has never been visited before
                    if True not in [self.thresholding(nodes.position, position) for nodes in self.tree]:
                        # create new node at position, add it to the tree and as a child of the previous node
                        # and remain in state 1
                        n = Node(position)
                        self.tree[n] = []
                        self.tree[self.priornode].append(n)
                        self.priornode = n
                        self.priorwhereat = 0
                        self.actions = [1] + self.actions
                        # if previously at a junction, output to rover to go forward
                        if self.priorwhereat == 1:
                            self.step_forward()
                    # check if we have previously visited this node
                    else:
                        # find the node in the tree and visit it, then go to state 4[0]
                        for node in self.tree:
                            if self.thresholding(node.position, position):
                                node.visit()
                        self.actions = [[4, self.priornode]] + self.actions
                # check if at junction
                elif whereat == 1:
                    # check if not previously visited
                    if True not in [self.thresholding(nodes.position, position) for nodes in self.tree]:
                        # go to state 2 and tell rover to idle
                        self.actions = [2] + self.actions
                        self.idle()
                    # check if previously visited
                    else:
                        # find node in tree, visit it and go back to prior node
                        for node in self.tree:
                            if self.thresholding(node.position, position):
                                node.visit()
                        self.actions= [[4,self.priornode]] + self.actions
                # check if at dead end; if true, go to state 4[0] and idle
                elif whereat == 2:
                    self.actions = [[4, self.priornode]] + self.actions
                    self.idle()
                # check if at exit; if true, create end node and go to state 4[0]
                elif whereat == 3:
                    n = Node(position)
                    self.tree[n] = []
                    n.setend()
                    self.tree[self.priornode].append(n)
                    self.priornode = n
                    self.actions = [[4, self.priornode]] + self.actions
            # check if in state 2; if true, go to state 3 and output to rover to spin
            elif currentAction == 2:
                self.actions = [3] + self.actions
                self.spin()
            # check if in state 3
            elif currentAction == 3:
                # if position was previously visited, find node in tree, visit it and go to state 4[0]
                if True in [self.thresholding(nodes.position, position) for nodes in self.tree]:
                    for node in self.tree:
                        if self.thresholding(node.position, position):
                            node.visit()
                    self.actions = [[4, self.priornode]] + self.actions
                # check if position not previously visited
                else:
                    # triangulate position
                    newx, newy = triangulate(beaconangles[0], beaconangles[1], beaconangles[2])
                    self.updatePos(newx, newy)
                    
                    # create new node
                    n = Node(newx, newy)
                    self.tree[n] = []
                    self.tree[self.priornode].append(n)
                    
                    # tell rover to eventually go to state 4[0]
                    self.actions = [[4, self.priornode]] + self.actions
                    self.priornode = n
                    self.priorwhereat = 1
                    # tell rover to visit each branch and then return to the junction. done by adding state 3[0] and 4[0] to state machine 
                    for i in potentialbranches:
                        self.actions = [[3, i], [4, self.priornode]] + self.actions 
            # check if in state 3[0]; if true, go to state 6[0], then state 1
            elif currentAction[0] == 3:
                self.actions = [[6,currentAction[1]], 1] + self.actions
            # check if in state 4[0]; if true, visit node and tell rover to go back to the previous node
            elif currentAction[0] == 4:
                currentAction[1].visit()
                self.go_back(currentAction[1], orientation)
            # check if in state 6[0]; if true, tell rover to set its angle and step forward
            elif currentAction[0] == 6:
                self.setAngle(currentAction[1], position)
                self.step_forward()
            # check if in state 7[0]
            elif currentAction[0] == 7:
                # if the rover has not returned to its prior position, return to state 7[0]
                if not self.thresholding(position, currentAction[1].position):
                    self.actions = [[7, currentAction[1]]] + self.actions
        # if no more actions left, rover is finished
        else:
            self.toreturn.append(5)

        return self.toreturn

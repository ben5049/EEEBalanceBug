# Aranya Gupta
# Started 18/5/2023, major revision 31/5/2023
from triangulate import triangulate
from time import time
from math import degrees, atan
THRESHOLD = 50
PREVIOUS_NODE_THRESHOLD = 150
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
    priornode = 0 # prior node for tremaux's algorithm to work 
    previouslyPlacedNode = 0 # the node most recently placed
    priorwhereat = 0
    sessionId = -1
    lastSeen = 0
    estop = False 
    startup = 0
    pause = False
    watchdog = 0

    def __init__(self, position, whereat, name):
        self.name = name
        n = Node(position)
        self.tree[n] = []
        self.priornode = n
        self.previouslyPlacedNode = n
        self.startup = int(time())
        if whereat == 0:
            self.actions.append(1)
            self.priorwhereat = 1
        elif whereat == 1:
            self.actions.append(2)
        self.toreturn = [-1]
        self.pause = True
        # return 200 ok response 
    
    def __str__(self):
        return str(self.name)
    
    def thresholding(self, pos1, pos2):
        if pos1[0] == 0 and pos1[1] == 0 and pos2[0] == 0 and pos2[1]==0:
            print("0,0 EXCEPTION")
            return False
        if abs(self.priornode.position[0] - pos2[0]) < PREVIOUS_NODE_THRESHOLD and abs(self.priornode.position[1] - pos2[1]) < PREVIOUS_NODE_THRESHOLD:
            print("NEAR PRIOR NODE")
            return False
        if abs(self.previouslyPlacedNode.position[0] - pos2[0]) < PREVIOUS_NODE_THRESHOLD and abs(self.previouslyPlacedNode.position[1] - pos2[1]) < PREVIOUS_NODE_THRESHOLD:
            print("NEAR PREVIOUSLY PLACED NODE")
            return False   
        if abs(pos1[0] -pos2[0]) < THRESHOLD and abs(pos1[1] -pos2[1]) < THRESHOLD:
            print("FINALLY TRUE")
            return True
        print("ELSE ISSUE")
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
    
    def go_back(self, node, position, previouslyvisited):
        print("CURRENT POSITION:", position)
        print("GOING TO:", node)
        try:
            if (previouslyvisited.position[0]-position[0]) == 0:
                if (previouslyvisited.position[1]-position[1]) == 0:
                    angle = 0
                else:
                    angle = 180    
            else:
                angle = degrees(atan((previouslyvisited.position[0]-position[0])/(previouslyvisited.position[1]-position[1])))+180
        except:
            if (previouslyvisited.position[0]-position[0]) > 0:
                angle = 90
            else:
                angle = -90

        self.setAngle(angle)
        self.step_forward()
        self.actions = [[7, node]] + self.actions
        
    def idle(self):
        self.toreturn.append(3)
    
    def updatePos(self, newx, newy):
        self.toreturn.append(4)
        self.toreturn.append(float(newx))
        self.toreturn.append(float(newy))

    def tremaux(self, position, whereat, potentialbranches, beaconangles):
        # if paused, remain idle
        if self.pause:
            return [3]
        
        # special state when rover has just started - tell it to move forward
        if len(self.toreturn)!=0:
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
            if len(self.actions)!=0:
                if currentAction == self.actions[0]:
                    currentAction = self.actions.pop(0)
            # if in state 1
            if currentAction == 1:
                if whereat == 0:
                    if True not in [self.thresholding(nodes.position, position) for nodes in self.tree]:
                        # if at passage, go forward until we reach a junction
                        n = Node(position)
                        self.tree[self.priornode].append(n)
                        self.tree[self.previouslyPlacedNode].append(n)
                        
                        self.priorwhereat = 0
                        self.previouslyPlacedNode = n
                        self.actions = [1] + self.actions
                    else:
                        self.actions = [[4, self.priornode]] + self.actions
                        self.idle()
                elif whereat == 1:
                    # if at junction, go to spinning state, and idle
                    if True not in [self.thresholding(nodes.position, position) for nodes in self.tree]:
                        self.actions = [2] + self.actions
                        self.idle()
                    else:
                        self.actions= [[4,self.priornode]] + self.actions
                elif whereat == 2:
                    # if at dead end, go back
                    self.actions = [[4, self.priornode]] + self.actions
                    self.idle()
                elif whereat == 3:
                    pass
                elif whereat == 4:
                    # if about to exit a passage, keep going
                    self.actions = [1] + self.actions
                    self.step_forward()
            elif currentAction == 2:
                if whereat == 4:
                    # if about to exit a passage, keep going
                    self.actions = [1] + self.actions
                    self.step_forward()
                else:
                    self.actions = [3] + self.actions
                    self.spin()

            elif currentAction == 3:
                if True in [self.thresholding(nodes.position, position) for nodes in self.tree]:
                    self.actions = [[4, self.priornode]] + self.actions
                else:
                    if whereat == 0 or whereat == 1 or whereat == 2 or whereat == 3:
                        if len(beaconangles)==3 and len(potentialbranches)!=0:
                            # triangulate position
                            if beaconangles[0] == beaconangles[1] and beaconangles[1]==beaconangles[2] and beaconangles[1]==0:
                                newx, newy = position[0], position[1]
                            else:
                                try:
                                    newx, newy = triangulate(beaconangles[0], beaconangles[1], beaconangles[2])
                                except:
                                    newx, newy = position[0], position[1]
                            self.updatePos(newx, newy)
                            # create new node

                            n = Node((newx, newy))
                            self.tree[n] = []
                            self.tree[self.priornode].append(n)

                            # tell rover to eventually go to state 4[0]
                            self.actions = [[4, self.priornode]] + self.actions
                            self.priornode = n
                            self.previouslyPlacedNode = n
                            self.priorwhereat = 1
                            # tell rover to visit each branch and then return to the junction. done by adding state 3[0] and 4[0] to state machine 
                            for i in potentialbranches:
                                self.actions = [[3, i], [4, self.priornode]] + self.actions 
                        else:
                            self.actions = [3] + self.actions
                    elif whereat == 4:
                        self.step_forward()
                        self.actions = [1] + self.actions
            elif currentAction[0] == 3:
                if whereat == 0 or whereat == 1 or whereat == 2 or whereat == 3:
                    self.setAngle(currentAction[1])
                    self.step_forward()
                    self.toreturn = [1] + self.toreturn
                elif whereat == 4:
                    self.step_forward()
                    self.actions = [1] + self.actions 

            elif currentAction[0] == 4:
                if whereat == 1 or whereat == 2 or whereat == 3:
                    self.go_back(self.priornode, position, self.previouslyPlacedNode)
                elif whereat == 0 or whereat == 4:
                    self.step_forward()
                    self.actions = [1] + self.actions 
            
            elif currentAction[0] == 7:
                # if the rover has not returned to its prior position, return to state 7[0]
                if not self.thresholding(currentAction[1].position, position):
                    self.actions = [[7, currentAction[1]]] + self.actions
                
                    
            
        # if no more actions left, rover is finished
        else:
            self.toreturn.append(5)

        return self.toreturn

# 0,0 test

# line test
# r = Rover( (0,0), 0, "tits")
# print("STATES: ", r.actions)
# print("ACTIONS: ", r.tremaux((0,0), 0, [], []))
# r.pause = False
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((0,100), 0, [], []))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((0,200), 0, [], []))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((0,300), 0, [], []))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((0,400), 0, [], []))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((0,500), 2, [], []))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((0,500), 0, [], []))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((0,400), 0, [], []))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((0,300), 0, [], []))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((0,100), 0, [], []))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((0,000), 2, [], []))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((0,000), 2, [], []))




# crossroads test


# circle test
# r = Rover( (0,0), 0, "tits",)
# print("STATES: ", r.actions)
# print("ACTIONS: ", r.tremaux((0,0), 0, [], [], 0))
# r.pause = False
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((0,100), 0, [], [], 0))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((0,200), 0, [], [], 0))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((0,300), 0, [], [], 0))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((0,400), 1, [], [], 0))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((0,400), 1, [], [], 0))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((0,400), 1, [], [], 0))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((0,400), 1, [], [], 0))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((0,400), 1, [90], [0,0,0], 0))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((0,400), 1, [], [], 0))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((0,400), 1, [], [], 0))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((100,400), 0, [], [], 90))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((200,400), 0, [], [], 90))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((300,400), 0, [], [], 90))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((400,400), 1, [], [], 90))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((400,400), 1, [], [], 90)) # spins here
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((400,400), 1, [], [], 90))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((400,400), 1, [], [], 90))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((400,400), 1, [], [], 90))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((400,400), 1, [180], [0, 0, 0], 90))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((400,400), 1, [], [], 90))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((400,400), 1, [], [], 90))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((400,300), 0, [], [], 180))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((400,200), 0, [], [], 180))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((400,100), 0, [], [], 180))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((400,0), 1, [], [], 180))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((400,0), 1, [], [], 180)) # spins here
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((400,0), 1, [], [], 180))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((400,0), 1, [], [], 180))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((400,0), 1, [-90], [0, 0, 0], 180))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((400,0), 1, [], [], 180))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((400,0), 1, [], [], -90))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((300,0), 0, [], [], -90))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((200,0), 0, [], [], -90))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((100,0), 0, [], [], -90))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((0,0), 0, [], [], -90))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((0,0), 0, [], [], -90))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((100,0), 0, [], [], 90))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((300,0), 0, [], [], 90))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((400,0), 0, [], [], 90))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((400,0), 0, [], [], 90))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((400,0), 0, [], [], -90))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((400,400), 0, [], [], -90))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((400,400), 0, [], [], -90))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((0,400), 0, [], [], -90))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((0,400), 0, [], [], -90))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((0,0), 0, [], [], 0))
# print("STATES: ", r.actions)
# print("ACTIONS: ",r.tremaux((0,0), 0, [], [], 0))
# print("STATES: ", r.actions)

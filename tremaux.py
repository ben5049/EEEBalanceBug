# Aranya Gupta
# 19/5/2023

class Node:
    # state is either 1 or 2 - 1 being visited once, 2 being visited twice
    # when visited twice, it is treated as a "dead end" and will not be 
    # re-traversed, but will be added to children of prior node
    state = 0
    position = 0
    def __init__(self, position):
        self.position = position
        self.state = 1

    def visit(self):
        self.state = 2

# abstract funcs - to be implemented later. will use recursion with tremaux()
# so that it is a depth-first search. dependent on how tremaux alg will control 
# rover

def step_forward():
    return

def go_back(priornode):
    return

def visit_left(priornode):
    return

def visit_right(priornode):
    return

def visit_straight(priornode):
    return


# position is x, y from starting position ie dead reckoning
# whereat is passage - 0, junc - 1
# whereat may be done on this side rather than on fpga 
#
# currently, after each movement, relevant data is sent to program
# which then uses tremaux to find the next step the rover should take

# tree represented as hash table with each node having its children as an array of values
nodes = {}
priorwhereat = 0
def tremaux(position, whereat, priornode):
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
                        return go_back(priornode)
    if whereat == 1:
        for node in nodes:
            if node.position == position:  #if we've already visited this node mark it as dead and go back
                node.visit()
                return go_back(priornode)
            
        # if we haven't visited this node, check all possible other routes, then backtrack
        n = Node(position)
        nodes[n] = []
        visit_left(n)
        visit_right(n)
        visit_straight(n)
        n.visit()
        go_back(priornode)
        

    priorwhereat = whereat
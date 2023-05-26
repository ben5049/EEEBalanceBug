from tremaux import Node
from graphviz import Digraph
import pygame
from math import cos, sin, pi

# backup for if actual graph visualiser doesn't work
# just displays graph showing interconnections between nodes
def graphVisualiser(graph):
    gra = Digraph()
    [gra.node(str(i)) for i in graph]
    for node1 in graph:
        for node2 in graph:
            if node2 in graph[node1]:
                gra.edge(str(node1), str(node2))
    gra.render('testGraph.gv.pdf', view=True)

# actual visualiser
# takes position, whereat, orientation data real time and builds walls around position
# how? distance from rover to wall will be measured (provisionally set to 30px)
# orientation is 0-359 (bearing from north)
# screen is pygame screen 
# needed as a class so that python passes it by reference, not value, and will update in tremaux
class Visualiser:
    screen = 0
    def __init__(self):
        pygame.init()
        self.screen = pygame.display.set_mode([500,500])
    
    def visualiser(self, position, whereat, orientation):
        if position!=None and whereat!=None and orientation!=None:
            # at passage, draw straight parallel lines next to user
            r = 30
            l = 30
            theta = orientation*pi/180
            pygame.draw.circle(self.screen, (255,255,255), position, 3)
            if whereat == 0:
                pygame.draw.line(self.screen, (255,255,255), (position[0]+r*cos(theta), position[1]+r*sin(theta)), (position[0]+r*cos(theta)+l*sin(theta), position[1]+r*sin(theta)-l*cos(theta)))
                pygame.draw.line(self.screen, (255,255,255), (position[0]-r*cos(theta), position[1]-r*sin(theta)), (position[0]-r*cos(theta)+l*sin(theta), position[1]-r*sin(theta)-l*cos(theta)))
            # at junction, draw crossroads
            elif whereat == 1:
                # left junc
                pygame.draw.line(self.screen, (255,255,255), (position[0]-r*cos(theta), position[1]-r*sin(theta)), (position[0]-r*cos(theta)-l*cos(theta), position[1]-r*sin(theta)-l*(sin(theta))))
                pygame.draw.line(self.screen, (255,255,255), (position[0]-r*cos(theta)+2*r*sin(theta), position[1]-r*sin(theta)-2*r*cos(theta)), (position[0]-r*cos(theta)+2*r*sin(theta)-l*cos(theta), position[1]-r*sin(theta)-2*r*cos(theta)-l*sin(theta)))
                # right junc
                pygame.draw.line(self.screen, (255,255,255), (position[0]+r*cos(theta), position[1]+r*sin(theta)), (position[0]+r*cos(theta)+l*cos(theta), position[1]+r*sin(theta)+l*sin(theta)))
                pygame.draw.line(self.screen, (255,255,255), (position[0]+r*cos(theta)+2*r*sin(theta), position[1]+r*sin(theta)-2*r*cos(theta)), (position[0]+r*cos(theta)+2*r*sin(theta)+l*cos(theta), position[1]+r*sin(theta)-2*r*cos(theta)+l*sin(theta)))
                # straight junc
                pygame.draw.line(self.screen, (255,255,255), (position[0]-r*cos(theta)+2*r*sin(theta), position[1]-r*sin(theta)-2*r*cos(theta)), (position[0]-r*cos(theta)+2*r*sin(theta)+l*sin(theta), position[1]-r*sin(theta)-2*r*cos(theta)-l*cos(theta)))
                pygame.draw.line(self.screen, (255,255,255), (position[0]+r*cos(theta)+2*r*sin(theta), position[1]+r*sin(theta)-2*r*cos(theta)), (position[0]+r*cos(theta)+2*r*sin(theta)+l*sin(theta), position[1]+r*sin(theta)-2*r*cos(theta)-l*cos(theta)))
            elif whereat == 2:
                pygame.draw.line(self.screen, (255,255,255), (position[0]-r*cos(theta), position[1]-r*sin(theta)), (position[0]+r*cos(theta), position[1]+r*sin(theta)))
            elif whereat == 3:
                pygame.draw.line(self.screen, (0,255,0), (position[0]+r*cos(theta), position[1]+r*sin(theta)), (position[0]+r*cos(theta)+l*sin(theta), position[1]+r*sin(theta)-l*cos(theta)))
                pygame.draw.line(self.screen, (0,255,0), (position[0]-r*cos(theta), position[1]-r*sin(theta)), (position[0]-r*cos(theta)+l*sin(theta), position[1]-r*sin(theta)-l*cos(theta)))
            pygame.display.update()
            input()
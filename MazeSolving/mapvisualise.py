from tremaux import Node
from graphviz import Digraph
import pygame


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
# orientation is 0-right,1-left,2-up,3-down as seen from top-down view
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
            pygame.draw.circle(self.screen, (255,255,255), position, 3)
            if whereat == 0:
                if orientation==0:
                    pygame.draw.line(self.screen, (255,255,255), (position[0], position[1]-r), (position[0]+l, position[1]-r))
                    pygame.draw.line(self.screen, (255,255,255), (position[0], position[1]+r), (position[0]+l, position[1]+r))
                elif orientation==1:
                    pygame.draw.line(self.screen, (255,255,255), (position[0], position[1]-r), (position[0]-l, position[1]-r))
                    pygame.draw.line(self.screen, (255,255,255), (position[0], position[1]+r), (position[0]-l, position[1]+r))
                elif orientation==2:
                    pygame.draw.line(self.screen, (255,255,255), (position[0]-r, position[1]), (position[0]-r, position[1]-l))
                    pygame.draw.line(self.screen, (255,255,255), (position[0]+r, position[1]), (position[0]+r, position[1]-l))
                else:
                    pygame.draw.line(self.screen, (255,255,255), (position[0]-r, position[1]), (position[0]-r, position[1]+l))
                    pygame.draw.line(self.screen, (255,255,255), (position[0]+r, position[1]), (position[0]+r, position[1]+l))

            # at junction, draw crossroads
            elif whereat == 1:
                if orientation==0:
                    # left-facing junction
                    pygame.draw.line(self.screen, (255,255,255), (position[0], position[1]-r), (position[0], position[1]-r-l))
                    pygame.draw.line(self.screen, (255,255,255), (position[0]+2*r, position[1]-r), (position[0]+2*r, position[1]-r-l))
                    # right-facing junction
                    pygame.draw.line(self.screen, (255,255,255), (position[0], position[1]+r), (position[0], position[1]+r+l))
                    pygame.draw.line(self.screen, (255,255,255), (position[0]+2*r, position[1]+r), (position[0]+2*r, position[1]+r+l))              
                    # forward-facing junction
                    pygame.draw.line(self.screen, (255,255,255), (position[0]+2*r, position[1]-r), (position[0]+2*r+l, position[1]-r))
                    pygame.draw.line(self.screen, (255,255,255), (position[0]+2*r, position[1]+r), (position[0]+2*r+l, position[1]+r))
                elif orientation==1:
                    # left-facing junction
                    pygame.draw.line(self.screen, (255,255,255), (position[0], position[1]+r), (position[0], position[1]+r+l))
                    pygame.draw.line(self.screen, (255,255,255), (position[0]-2*r, position[1]+r), (position[0]-2*r, position[1]+r+l))
                    # right-facing junction
                    pygame.draw.line(self.screen, (255,255,255), (position[0], position[1]-r), (position[0], position[1]-r-l))
                    pygame.draw.line(self.screen, (255,255,255), (position[0]-2*r, position[1]-r), (position[0]-2*r, position[1]-r-l))
                    # forward-facing junction
                    pygame.draw.line(self.screen, (255,255,255), (position[0]-2*r, position[1]-r), (position[0]-2*r-l, position[1]-r))
                    pygame.draw.line(self.screen, (255,255,255), (position[0]-2*r, position[1]+r), (position[0]-2*r-l, position[1]+r))              
                elif orientation==2:
                    # left-facing junction
                    pygame.draw.line(self.screen, (255,255,255), (position[0]-r, position[1]-2*r), (position[0]-r-l, position[1]-2*r))
                    pygame.draw.line(self.screen, (255,255,255), (position[0]-r, position[1]), (position[0]-r-l, position[1]))              
                    # right-facing junction
                    pygame.draw.line(self.screen, (255,255,255), (position[0]+r, position[1]-2*r), (position[0]+r+l, position[1]-2*r))
                    pygame.draw.line(self.screen, (255,255,255), (position[0]+r, position[1]), (position[0]+r+l, position[1]))
                    # forward-facing junction
                    pygame.draw.line(self.screen, (255,255,255), (position[0]-r, position[1]-2*r), (position[0]-r, position[1]-2*r-l))
                    pygame.draw.line(self.screen, (255,255,255), (position[0]+r, position[1]-2*r), (position[0]+r, position[1]-2*r-l))
                else:
                    # left-facing junction
                    pygame.draw.line(self.screen, (255,255,255), (position[0]+r, position[1]), (position[0]+r+l, position[1]))
                    pygame.draw.line(self.screen, (255,255,255), (position[0]+r, position[1]+2*r), (position[0]+r+l, position[1]+2*r))
                    # right-facing junction
                    pygame.draw.line(self.screen, (255,255,255), (position[0]-r, position[1]), (position[0]-r-l, position[1]))
                    pygame.draw.line(self.screen, (255,255,255), (position[0]-r, position[1]+2*r), (position[0]-r-l, position[1]+2*r))
                    # forward-facing junction
                    pygame.draw.line(self.screen, (255,255,255), (position[0]-r, position[1]+2*r), (position[0]-r, position[1]+2*r+l))
                    pygame.draw.line(self.screen, (255,255,255), (position[0]+r, position[1]+2*r), (position[0]+r, position[1]+2*r+l))
                    
            # at dead end, draw vertical line
            elif whereat == 2:
                if orientation==0 or orientation==1:
                    pygame.draw.line(self.screen, (255,255,255), (position[0], position[1]+r), (position[0], position[1]-r))
                elif orientation==2 or orientation==3:
                    pygame.draw.line(self.screen, (255,255,255), (position[0]-r, position[1]), (position[0]+r, position[1]))

            elif whereat == 3:
                if orientation==0:
                    if orientation==0:
                        pygame.draw.line(self.screen, (0,255,0), (position[0], position[1]-r), (position[0]+l, position[1]-r))
                        pygame.draw.line(self.screen, (0,255,0), (position[0], position[1]+r), (position[0]+l, position[1]+r))
                    elif orientation==1:
                        pygame.draw.line(self.screen, (0,255,0), (position[0], position[1]-r), (position[0]-l, position[1]-r))
                        pygame.draw.line(self.screen, (0,255,0), (position[0], position[1]+r), (position[0]-l, position[1]+r))
                    elif orientation==2:
                        pygame.draw.line(self.screen, (0,255,0), (position[0]-r, position[1]), (position[0]-r, position[1]-l))
                        pygame.draw.line(self.screen, (0,255,0), (position[0]+r, position[1]), (position[0]+r, position[1]-l))
                    else:
                        pygame.draw.line(self.screen, (0,255,0), (position[0]-r, position[1]), (position[0]-r, position[1]+l))
                        pygame.draw.line(self.screen, (0,255,0), (position[0]+r, position[1]), (position[0]+r, position[1]+l))
            pygame.display.update()
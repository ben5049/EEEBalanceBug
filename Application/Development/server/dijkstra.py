# Aranya Gupta 
# 21/5/2023
from tremaux import Node    

# makes sure graph is bidirectional - if B is a neighbour of A, 
# A must be a neighbour of B 
def assertValid(graph):
    for node in graph:
        for neighbour in graph[node]:
            if node not in graph[neighbour]:
                graph[neighbour].append(node)
    return graph


# just because it looks ugly otherwise
def findDist(node1, node2):
    return ((node1[0] - node2[0])**2 + (node1[1] - node2[1])**2)**0.5

THRESHOLD = 10

# graph is hash table of Node as defined in tremaux 
# each Node has its position, Dijkstra will calculate 
# distances based on position
# returns predecessor graph showing shortest path from startNode to every point

def thresholding(pos1, pos2):
        if abs(pos1[0] -pos2[0]) < THRESHOLD and abs(pos1[1] -pos2[1]) < THRESHOLD:
            return True
        else:
            return False

def dijkstra(graph, startPos):
    for node in graph:
        if thresholding(startPos, node):
            startNode = node
            break
    G = {startNode: 0} # distance from startNode to given node
    P = {startNode: None} # predecessor graph - shortest path from given node to start node
    for node in graph:
        if node not in G:
            G[node] = 1e7
            P[node] = []
    
    while len(G)!=0:
        current = min(G, key=G.get)
        neighbours = graph[current]
        for neighbour in neighbours:
            dist = findDist(current, neighbour)+G[current]
            if neighbour in G and dist < G[neighbour]:
                G[neighbour] = dist
                P[neighbour] = current
        del G[current]
    
    return P


"""
Some tester code, if needed for debugging
a = Node((0,0)) 
b = Node((3,0)) 
c = Node((7,0)) 
d = Node((2.969,7.429)) 
e = Node((5,-1)) 

graph = {
    a:[b,c,d],
    b:[a,e,c],
    c: [a,b,e,d],
    d:[a,c],
    e:[b,c]
}

graph = assertValid(graph)
P = dijkstra(graph, b)
print([str(i.position) + ":" + str(P[i]) for i in P])
"""
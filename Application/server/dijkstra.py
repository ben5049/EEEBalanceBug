# Aranya Gupta 
# Started 21/5/2023

# makes sure graph is bidirectional - if B is a neighbour of A, 
# A must be a neighbour of B 
def assertValid(graph):
    # makes each element a tuple not an array
    for node in graph:
        for i in range(len(graph[node])):
            if type(graph[node][i])==list:
                graph[node][i] = (graph[node][i][0], graph[node][i][1])
                
    for node in graph:
        for neighbour in graph[node]:
            if node not in graph[neighbour]:
                graph[neighbour].append(node)
    return graph


# just because it looks ugly otherwise
def findDist(node1, node2):
    return ((node1[0] - node2[0])**2 + (node1[1] - node2[1])**2)**0.5


# graph is hash table of x-y positions
# Dijkstra will calculate distances based on position
# returns predecessor graph showing shortest path from startNode to every point
def dijkstra(graph, startPos):
    # finds node closest to start
    minDist = 1e9
    startNode = (0,0)
    for node in graph:
        dist = findDist(startPos, node)
        if  dist < minDist:
            minDist = dist
            startNode = node
    # attempts to initialise graphs - if it fails, returns none
    try:
        G = {startNode: 0} # distance from startNode to given node
        P = {startNode: None} # predecessor graph - shortest path from given node to start node
    except:
        return None

    # sets up large distances and predecessor graph 
    for node in graph:
        if node not in G:
            G[node] = 1e7
            P[node] = []
    # checks if all nodes have been visited
    while len(G)!=0:
        # finds node with current shortest distance from start, and its neighbours
        current = min(G, key=G.get)
        neighbours = graph[current]
        # finds the minimum distance from current point to neighbours
        for neighbour in neighbours:
            dist = findDist(current, neighbour)+G[current]
            # updates values in G and P if shorter distance found
            if neighbour in G and dist < G[neighbour]:
                G[neighbour] = dist
                P[neighbour] = current
        del G[current]
    
    return P

# puts predecessor graph found by dijkstra into json-friendly format
def formatPredecessor(P):
    d = {}
    i=0
    for key in P:
        d[i] = {"mapsto":-1, "xcoord":key[0], "ycoord":key[1]}
        i+=1
    for key in P:
        pred = P[key] #pred is (0,0), (100,0)
        prednode = -1
        for node in d:
            if pred is None:
                pass
            elif len(pred)==0:
                pass
            elif d[node]["xcoord"] == pred[0] and d[node]["ycoord"] == pred[1]:
                prednode = node
        for node in d:
            if key[0] == d[node]["xcoord"] and key[1]==d[node]["ycoord"]:
                d[node]["mapsto"] = prednode
    
    return d
    
        

    # d = {0:{mapsto:-1, xcoord:0, ycoord:0}, 1:{mapsto:0, xcoord:100, ycoord:0}, 2:{mapsto:1, xcoord:100, ycoord:100}, 3:{mapsto:0, xcoord:50, ycoord:50}}



# Some tester code, if needed for debugging

# graph = {
#     (0,0):[(100,0),(50,50)],
#     (50,50):[(0,0)],
#     (100,0): [(0,0),(100,100)],
#     (100,100):[(100,0)],
# }

# graph = assertValid(graph)
# P = dijkstra(graph, (3,0))
# print(P)
# P = formatPredecessor(P)
# print(P)

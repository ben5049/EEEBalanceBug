import tremaux, mapvisualise, dijkstra

# tremaux to map out maze
# visualise concurrently
# once finished, dijkstra to find shortest path
# visualise shortest path and take shortest path

def main(): 
    start = tremaux.Node((0,0))
    V = mapvisualise.Visualiser()
    nodes = {}
    graph = tremaux.tremaux(start, 0, 0, start, 0, V, nodes)
    graph = dijkstra.assertValid(graph)
    shortestGraph = dijkstra.dijkstra(graph, start)
    
    end = 0
    path = []
    for node in shortestGraph:
        if node.state == 3:
            end = node
            break
    
    while end != start:
        path.append(end)
        end = shortestGraph[end]
    path = path[::-1]

    position= tremaux.dataRequest(V)[0]
    xdiff = i.position[0]-position[0]
    ydiff = i.position[1]-position[1]
    angle = tremaux.degrees(tremaux.atan(ydiff/xdiff))
    for i in path:
        while xdiff > 0:
            position = tremaux.dataRequest(V)[0]
            tremaux.setAngle(angle)
            tremaux.step_forward()
            xdiff = i.position[0]-position[0]
            ydiff = i.position[1]-position[1]
            angle = tremaux.degrees(tremaux.atan(ydiff/xdiff))
    
main()
    

    

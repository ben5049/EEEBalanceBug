import tremaux, dijkstra

# tremaux to map out maze
# visualise concurrently
# once finished, dijkstra to find shortest path
# visualise shortest path and take shortest path

def main(): 
    rover = tremaux.Rover((0,0), 0, "MAC ADDRESS")
    next_command = rover.tremaux((0,0), 0, None, None)
    print(next_command)
    next_command = rover.tremaux((100,0), 0, None, None)
    print(next_command)
    next_command = rover.tremaux((200,0), 1, None, None)
    print(next_command[0])
    graph = next_command[1]
    print(graph)
    graph = dijkstra.assertValid(graph)
    start = 0
    for n in graph:
        start = n
        return
    shortestGraph = dijkstra.dijkstra(graph, start)
    # end = 0
    # path = []
    # for node in shortestGraph:
    #     if node.state == 3:
    #         end = node
    #         break
    
    # while end != start:
    #     path.append(end)
    #     end = shortestGraph[end]
    # path = path[::-1]

    # position= tremaux.dataRequest(V)[0]
    # xdiff = i.position[0]-position[0]
    # ydiff = i.position[1]-position[1]
    # angle = tremaux.degrees(tremaux.atan(ydiff/xdiff))
    # for i in path:
    #     while xdiff > 0:
    #         position = tremaux.dataRequest(V)[0]
    #         tremaux.setAngle(angle)
    #         tremaux.step_forward()
    #         xdiff = i.position[0]-position[0]
    #         ydiff = i.position[1]-position[1]
    #         angle = tremaux.degrees(tremaux.atan(ydiff/xdiff))
    
main()

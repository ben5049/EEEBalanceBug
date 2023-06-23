#!/bin/bash
# Aranya Gupta
# Started 29/5/23
from flask import Flask, jsonify, request, make_response
from flask_cors import CORS
import tremaux, dijkstra
import mariadb
from time import time
from triangulate import triangulate
DEBUG = True

# Set up server

app = Flask(__name__)
cors = CORS(app, resources={r"/*":{"origins":"*"}})
commandQueue = [[3], [3], [3], [3], [3], [3], [3], [3], [3], [3], [3], [2, 0]]

# Server global variables
TIMEOUT = 5
LED_TIMEOUT = 30
rovers = []
isSpinning = False
spinTime = time()
hostip = '54.165.59.37'

# database set to run on port 3306, flask server set to run on port 5000 (when deploying, not developing)
try:
    conn = mariadb.connect(
        user='bb',
        password='',
        host=hostip,
        port=3306,
        database='BalanceBug'
    )
    print("Database connection set up")
except mariadb.Error as e:
    print(f"Error connecting to MariaDB platform: {e}")

cur = conn.cursor()

# default gateway to test if server is up
@app.route("/")
def hello():
    return jsonify({"hello":"world"})

@app.route("/set_angle", methods=["POST"])
def set_angle():
    data = request.get_json()
    global commandQueue
    commandQueue = [[3], [3], [3], [3], [3], [3], [3], [3], [3], [3], [3]]
    commandQueue.append([2, data["angle"]])
    print(commandQueue)
    return make_response(jsonify({"suc":"ces"}))

# Communication with rover
@app.route("/rover", methods=["POST"])
def rover():
    global conn, cur, demo, demo_beacons
    data = request.get_json() # data has keys "diagnostics", "MAC", "nickname", "timestamp", "position", "whereat", "orientation", "branches", "beaconangles", "tofleft", "tofright"
    r = 0
    flag = True
    # check if rover is already active
    for rover in rovers:
        if rover.name == data["MAC"]:
            r = rover
            flag = False
            break
    # Logic for if rover is not active
    if flag:
        # Create new rover instance
        r = tremaux.Rover(data["position"], data["whereat"], data["MAC"])
        r.nickname = data["nickname"]
        # Try to add rover into database, if its mac is new (ie never seen before rover)
        try:
            cur.execute("INSERT INTO Rovers (MAC, nickname) VALUES (? , ?)", (str(data["MAC"]), data["nickname"]))
        except mariadb.Error as e:
            # Try to find rover if it already exists in database 
            cur.execute("SELECT * FROM Rovers WHERE MAC=?", (str(data["MAC"]),))
            flag = True
            for mac, nick in cur:
                if str(mac) == str(data["MAC"]):
                    r.nickname = nick
                    flag = False

            # rover must either be brand new or already exist if not active, so this should never happen
            if flag:
                return make_response(jsonify({"error":"Specified MAC does not exist"}), 400) 
                           
        # Create a new session for the rover as it has just connected, and get and store its session id
        try:
            cur.execute("INSERT INTO Sessions (MAC,  SessionNickname) VALUES (?, ?)", (data["MAC"], r.startup+data["timestamp"]/1000))
            cur.execute("SELECT MAX(SessionID) FROM Sessions WHERE MAC=?", (data["MAC"],))
        except mariadb.Error as e:
            return make_response(jsonify({"error":f"Incorrectly formatted request: {e}"}), 400)
        for x in cur:
            r.sessionId = x[0]
        
        rovers.append(r)
    
    # create response to rover and reset timeout
    print(time()-r.lastSeen, "is", r, " last seen")
    r.lastSeen = time()
    if "beaconangles" not in data:
        data["beaconangles"] = []
    if "branches" not in data:
        data["branches"] = []
    resp = r.tremaux(data["position"], data["whereat"], data["branches"], data["beaconangles"])

    # spin  test
    # if len(data["beaconangles"]) == 3:
    #     resp.append(4)
    #     newx, newy = triangulate(data["beaconangles"][0], data["beaconangles"][1], data["beaconangles"][2])
    #     resp.append(newx)
    #     resp.append(newy)
    #     print(resp, "RESP")
    # print("YAW: ", data["diagnostics"]["connection"])

    # user input to resp
    # resp = []
    # user =int(input("Next command: "))
    # while user != -1:
    #     resp.append(user)
    #     user = int(input("Next command: "))

    #using command queue 
    # if (len(commandQueue))!=0:
    #     r.actions = []
    #     resp = []
    #     for i in commandQueue.pop(0):
    #         resp.append(i)
    # else:
    #     resp = []
    
    if demo:
        resp = [1]
        if len(data["beaconangles"])==3 and len(data["branches"])!=0:
            print(data["beaconangles"])==3 and len(data["branches"])
        demo_beacons = data["beaconangles"]

        

    resp = {"next_actions" : resp, "clear_queue":r.estop}
    # if rover is about to spin, set flags to turn on beacons
    if 1 in resp["next_actions"]:
        global isSpinning, spinTime
        isSpinning = True
        spinTime = time()
    
    # store positions and timestamp in database
    # also store tree in database if rover is done traversing
    try:
        cur.execute("INSERT INTO ReplayInfo (timestamp, xpos, ypos, whereat, orientation, tofleft, tofright, MAC, SessionID) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)", (r.startup+data["timestamp"]/1000, data["position"][0], data["position"][1], data["whereat"], data["orientation"], data["tofleft"], data["tofright"], data["MAC"], r.sessionId))
    
        #cur.execute("INSERT INTO ReplayInfo (timestamp, xpos, ypos, whereat, orientation, tofleft, tofright, MAC, SessionID) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)", (r.startup+data["timestamp"]/1000, data["position"][0], data["position"][1], data["whereat"], data["orientation"], 1000, 1000, data["MAC"], r.sessionId))
        
        cur.execute("INSERT INTO Diagnostics (MAC, timestamp, battery, connection, SessionID) VALUES (?, ?, ?, ?, ?)", (data["MAC"], r.startup+data["timestamp"]/1000, data["diagnostics"]["battery"], data["diagnostics"]["connection"], r.sessionId))

        if 5 in resp["next_actions"]:
            for node in r.tree:
                neighbours = []
                for neighbour in r.tree[node]:
                    neighbours.append(str(neighbour))
                cur.execute("INSERT INTO Trees (SessionID, node_x, node_y, children) VALUES (?, ?, ?, ?)", (r.sessionId, node.position[0], node.position[1], str(neighbours)))
    except mariadb.Error as e:
        return make_response(jsonify({"error":f"Incorrectly formatted request: {e}"}), 400)
    
    if DEBUG:
        print(data)
        print("ACTIONS", r.actions)
        print(resp)
        print("\n")
    conn.commit()
    return make_response(jsonify(resp), 200)

# Give all rovers, active or inactive, to client
@app.route("/client/allrovers", methods=["GET"])
def allrovers():
    global conn, cur
    d = []
    disallowedMacs = []
    # add all active rovers
    for rover in rovers:
        temp = {}
        temp["MAC"] = rover.name
        disallowedMacs.append(rover.name)
        temp["nickname"] = rover.nickname
        temp["connected"] = True
        temp["sessionid"] = rover.sessionId
        d.append(temp)
        print(temp)
    
    # add all inactive rovers from database
    t = ""
    for i in disallowedMacs:
        t+="MAC != \""+str(i)+"\" AND "
    t = t[:-5]
    
    command = "SELECT * FROM Rovers WHERE "+t
    if command == "SELECT * FROM Rovers WHERE ":
        command = "SELECT * FROM Rovers" 
    try:
        cur.execute(command)
    except mariadb.Error as e:
        print(str(e))
        return make_response(jsonify({"error":f"Incorrectly formatted request: {e}"}), 400)
    for rover in cur:
        temp = {}
        temp["MAC"] = rover[0]
        temp["nickname"] = rover[1]
        temp["connected"] = False
        d.append(temp)

    return make_response(jsonify(d), 200)

# get a specific session replay
@app.route("/client/replay", methods=["POST"])
def replay():
    global conn, cur
    data = request.get_json()
    
    # get requested session id
    try:
        sessionid = int(data["sessionid"])
    except:
        return make_response(jsonify({"error":"Incorrectly formatted request: missing sessionid"}), 400)
    
    # get corresponding replay
    try:
        cur.execute("SELECT * FROM ReplayInfo WHERE SessionID=?", (sessionid,))
    except mariadb.Error as e:
        return make_response(jsonify({"error":f"Incorrectly formatted request: {e}"}), 400)

    if cur is None:
        return make_response(jsonify({"error":"Session does not exist"}), 404)
    
    # generate response
    resp = {}
    for timestamp, xpos, ypos, whereat, orientation, tofleft, tofright, mac, sessionid in cur:
        resp[timestamp] = [xpos, ypos, whereat, orientation, tofleft, tofright]

    return make_response(jsonify(resp))

# Get all sessions
@app.route("/client/sessions", methods=["GET"])
def sessions():
    global conn, cur
    cur.execute("SELECT * FROM Sessions ORDER BY SessionId DESC;")
    d = []

    for mac, sessionid, nickname in cur:
        temp = {}
        temp["sessionid"] = sessionid
        temp["MAC"] = mac
        temp["nickname"] = nickname
        d.append(temp)
    
    return make_response(jsonify(d), 200)

# Get diagnostic data from given session
@app.route("/client/diagnostics", methods=["POST"])
def diagnostics():
    global conn, cur
    data = request.get_json()
    
    try:
        cur.execute("SELECT * FROM Diagnostics WHERE SessionID=? ORDER BY timestamp DESC;", (data["sessionid"],))
    except:
        return make_response(jsonify({"error":"Incorrectly formatted request: missing/invalid sessionid"}), 400)
    
    d = []
    for mac, timestamp, battery, connection, sessionid in cur:
        t = {"MAC":mac, "timestamp":timestamp, "battery":battery, "connection":connection}
        d.append(t)
    return make_response(jsonify(d), 200)

# check if rover is timed out - if it is, remove it
@app.route("/timeout", methods=["GET"])
def timeout():
    for rover in rovers:
        if time()-rover.lastSeen > TIMEOUT:
            print(time()-rover.lastSeen, rover, " TIME SINCE LAST SEEN")
            for node in rover.tree:
                neighbours = []
                for neighbour in rover.tree[node]:
                    neighbours.append(str(neighbour))
                try:
                    cur.execute("INSERT INTO Trees (SessionID, node_x, node_y, children) VALUES (?, ?, ?, ?)", (rover.sessionId, node.position[0], node.position[1], str(neighbours)))
                except mariadb.Error as e:
                    pass
            rovers.remove(rover)

# Pause a given rover
@app.route("/client/pause", methods=["POST"])
def pause():
    global conn, cur
    data = request.get_json()
    
    try:
        mac = str(data["MAC"])
    except:
        return make_response(jsonify({"error":"Incorrectly formatted request: missing MAC"}), 400)
    
    flag = True
    for rover in rovers:
        if rover.name == mac:
            rover.pause = True
            flag = False

    if flag:
        return make_response(jsonify({"error":"Selected rover does not exist, or is not currently connected"}), 400)
    
    return make_response(jsonify({"success":"successfully paused rover"}), 200)

# Play a given rover
@app.route("/client/play", methods=["POST"])
def play():
    global conn, cur
    data = request.get_json()
    
    try:
        mac = str(data["MAC"])
    except:
        return make_response(jsonify({"error":"Incorrectly formatted request: missing MAC"}), 400)
    
    flag = True
    for rover in rovers:
        if rover.name == mac:
            rover.pause = False
            flag = False
    
    if flag:
        return make_response(jsonify({"error":"Selected rover does not exist on play, or is not currently connected"}), 400)

    return make_response(jsonify({"success":"successfully played rover"}), 200)

# Set a nickname for a session
@app.route("/client/sessionnickname", methods=["POST"])
def sessionNickname():
    global conn, cur
    data = request.get_json()
    
    try:
        session = data["sessionid"]
        nick = data["sessionNick"]
    except:
        return make_response(jsonify({"error":"Incorrectly formatted request: missing sessionid or sessionNick"}), 400)
    
    try:
        cur.execute("UPDATE Sessions SET SessionNickname=? WHERE SessionId=?", (nick, session,))
    except mariadb.Error as e:
        return make_response(jsonify({"error":f"Incorrectly formatted request: {e}"}), 400)
    
    return make_response(jsonify({"success":"successfully changed session nickname"}), 200)

# Set rover nickname
@app.route("/client/rovernickname", methods=["POST"])
def addnickname():
    global conn, cur
    data = request.get_json()
    
    try:
        mac = data["MAC"]
        nick = data["nickname"]
    except:
        return make_response(jsonify({"error":"Incorrectly formatted request: missing MAC or nickname"}), 400)

    for rover in rovers:
        if rover.name == mac:
            rover.nickname = nick
    
    try:
        cur.execute("UPDATE Rovers SET nickname=? WHERE MAC=?", (nick, mac,))
    except mariadb.Error as e:
        return make_response(jsonify({"error":f"Incorrectly formatted request: {e}"}), 400)

    return make_response(jsonify({"success":"successfully changed rover nickname"}), 200)

# Get shortest path predecessor graph from stored trees
@app.route("/client/shortestpath", methods=["POST"])
def findShortestPath():
    global conn, cur
    data = request.get_json()
    
    try:
        sessionid = data["sessionid"]
        start_x = data["start_x"]
        start_y = data["start_y"]
    except:
        return make_response(jsonify({"error":"Incorrectly formatted request: missing sessionid or start"}), 400)
    
    tree = {}
    try:
        cur.execute("SELECT * FROM Trees WHERE SessionID=? ", (sessionid,))
    except:
        return make_response(jsonify({"error":f"Incorrectly formatted request: {e}"}), 400)
    
    if cur is None:
        return make_response(jsonify({"error":"Incorrectly formatted request: invalid sessionid"}), 400)

    # format tree so it is python-readable 
    for sid, node_x, node_y, children in cur:
        try:
            children = eval(children)
            tree[(node_x, node_y)] = [eval(x) for x in children]
        except:
            tree[(node_x, node_y)] = []
    
    # assert that a tree is valid and formatted bidirectionally
    tree = dijkstra.assertValid(tree)

    # get tree predecessor graph
    P = dijkstra.dijkstra(tree, [float(start_x), float(start_y)])
    print(P, "shortestPath predecessor non formatted")
    if P is None:
        return make_response(jsonify({"error":"Tree does not exist"}), 400)
    
    P = dijkstra.formatPredecessor(P)

    if DEBUG:
        print(tree, "shortestPath tree")
        print(P, "shortestPath predecessor")
    return make_response(jsonify(P), 200) # for testing

# emergency stops rover - must be disconnected and reconnected
@app.route("/client/estop", methods=["POST"])
def estop():
    global conn, cur
    data = request.get_json()
    
    try:
        mac = data["MAC"]
    except:
        return make_response(jsonify({"error":"Missing MAC address"}))
    
    flag = True
    for rover in rovers:
        if rover.name == mac:
            rover.estop = True
            flag = False
    
    if flag:
        return make_response(jsonify({"error":"Invalid MAC address"}))
    return make_response(jsonify({"success":"estopped"}))

red_override = 0
blue_override = 0
yellow_override = 0
# turns on or off red LED
@app.route("/led_driver/red", methods=["POST"])
def led_driver_red():
    global conn, cur
    global isSpinning, spinTime, red_override
    data = request.get_json()
    energy = data["energy status"]
    try:
        override = data["override_red"]
    except:
        pass
    
    try:
        if override:
            red_override = 1
            return make_response(jsonify({"success":"received data", "switch":override}), 200) #switch should be 1
        else:
            red_override = 0
            return make_response(jsonify({"success":"received data", "switch":override}), 200) #switch should be 1
    except:
        pass

    if red_override:
        return make_response(jsonify({"success":"received data", "switch":1}), 200)
    if isSpinning and time()-spinTime < LED_TIMEOUT and energy == "enough energy":
        red_status = 1
        return make_response(jsonify({"success":"received data", "switch":1}), 200) #switch should be 1
    elif energy != "enough energy":
        red_status = 0
        return make_response(jsonify({"success":"received data", "switch":0}), 200) # siwtch should be 0
    # logic to turn off led
    else:
        isSpinning = False
        red_status = 0
        return make_response(jsonify({"success":"received data", "switch":0}), 200) # swicth should be 0

@app.route("/led_driver/blue", methods=["POST"])
def led_driver_blue():
    global isSpinning, spinTime, blue_override
    data = request.get_json()
    energy = data["energy status"]
    try:
        override = data["override_blue"]
    except:
        pass

    try:
        if override:
            blue_override = 1
            return make_response(jsonify({"success":"received data", "switch":override}), 200) #switch should be 1
        else:
            blue_override = 0
            return make_response(jsonify({"success":"received data", "switch":override}), 200) #switch should be 1
    except:
        pass
    if blue_override:
        return make_response(jsonify({"success":"received data", "switch":1}), 200) #switch should be 1
    if isSpinning and time()-spinTime < LED_TIMEOUT and energy == "enough energy":
        return make_response(jsonify({"success":"received data", "switch":1}), 200)
    elif energy != "enough energy":
        return make_response(jsonify({"success":"received data", "switch":0}), 200) # siwtch should be 0
    else:
        isSpinning = False
        return make_response(jsonify({"success":"received data", "switch":0}), 200)

@app.route("/led_driver/yellow", methods=["POST"])
def led_driver_yellow():
    global isSpinning, spinTime, yellow_override
    data = request.get_json()
    energy = data["energy status"]
    try:
        override = data["override_yellow"]
    except:
        pass

    try:
        if override:
            yellow_override = 1
            return make_response(jsonify({"success":"received data", "switch":override}), 200) #switch should be 1
        else:
            yellow_override = 0
            return make_response(jsonify({"success":"received data", "switch":override}), 200) #switch should be 1
    except:
        pass
    if yellow_override:
        return make_response(jsonify({"success":"received data", "switch":1}), 200) #switch should be 1
    if isSpinning and time()-spinTime < LED_TIMEOUT and energy == "enough energy":
        return make_response(jsonify({"success":"received data", "switch":1}), 200)
    elif energy != "enough energy":
        return make_response(jsonify({"success":"received data", "switch":0}), 200) # siwtch should be 0'
    else:
        isSpinning = False
        return make_response(jsonify({"success":"received data", "switch":0}), 200)
    
demo = False
demo_beacons = []
@app.route("/demo_spin", methods=["POST"])
def demo_spin():
    global demo, demo_beacons
    demo = not demo
    return make_response(jsonify(demo_beacons), 200)

#---------------------ERROR HANDLING------------------------#
@app.errorhandler(400)
def bad_request(e):
    print(str(e))
    return make_response(jsonify({"error":str(e)}), 400)

@app.errorhandler(404)
def not_found(e):
    return make_response(jsonify({"error":str(e)}), 404)

@app.errorhandler(500)
def internal_error(e):
    return make_response(jsonify({"error":str(e)}), 500)
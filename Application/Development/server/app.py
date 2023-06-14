#!/bin/bash
# Aranya Gupta
# Started 29/5/23
from flask import Flask, jsonify, request, make_response
from flask_cors import CORS
import tremaux, dijkstra
import mariadb
from time import time
from json import loads
import logging
DEBUG = True

# Set up server
if DEBUG:
    logging.basicConfig(filename='record.log', level=logging.DEBUG)
app = Flask(__name__)
if DEBUG:
    app.logger.debug("Starting debug logging: ")
cors = CORS(app, resources={r"/*":{"origins":"*"}})

# Server global variables
TIMEOUT = 30
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

# Communication with rover
@app.route("/rover", methods=["POST"])
def rover():
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
        r.pause = True
        r.nickname = data["nickname"]
        # Try to add rover into database, if its mac is new (ie never seen before rover)
        try:
            cur.execute("INSERT INTO Rovers (MAC, nickname) VALUES (? , ?)", (str(data["MAC"]), data["nickname"]))
            conn.commit()
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
            cur.execute("INSERT INTO Sessions (MAC,  SessionNickname) VALUES (?, ?)", (data["MAC"], r.startup+int(data["timestamp"])))
            conn.commit()
            cur.execute("SELECT MAX(SessionID) FROM Sessions WHERE MAC=?", (data["MAC"],))
            conn.commit()
        except mariadb.Error as e:
            return make_response(jsonify({"error":f"Incorrectly formatted request: {e}"}), 400)
        for x in cur:
            r.sessionId = x[0]
        
        rovers.append(r)
    
    # create response, to rover, and reset timeout
    r.lastSeen = time()
    resp = r.tremaux(data["position"], data["whereat"], data["branches"], data["beaconangles"], data["orientation"])
    resp = {"next_actions" : resp, "clear_queue":r.estop}
    # if rover is about to spin, set flags to turn on beacons
    if 1 in resp["next_actions"]:
        global isSpinning, spinTime
        isSpinning = True
        spinTime = time()
    
    # store positions and timestamp in database
    # also store tree in database if rover is done traversing
    try:
        cur.execute("INSERT INTO ReplayInfo (timestamp, xpos, ypos, whereat, orientation, tofleft, tofright, MAC, SessionID) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)", (r.startup+int(data["timestamp"]), data["position"][0], data["position"][1], data["whereat"], data["orientation"], data["tofleft"], data["tofright"], data["MAC"], r.sessionId))
        conn.commit()
        cur.execute("INSERT INTO Diagnostics (MAC, timestamp, battery, connection, SessionID) VALUES (?, ?, ?, ?, ?)", (data["MAC"], r.startup+int(data["timestamp"]), data["diagnostics"]["battery"], data["diagnostics"]["connection"], r.sessionId))
        conn.commit()
        if 5 in resp["next_actions"]:
            for node in r.tree:
                neighbours = []
                for neighbour in r.tree[node]:
                    neighbours.append(str(neighbour))
                cur.execute("INSERT INTO Trees (SessionID, node_x, node_y, children) VALUES (?, ?, ?, ?)", (r.sessionId, node.position[0], node.position[1], str(neighbours)))
                conn.commit()
    except mariadb.Error as e:
        return make_response(jsonify({"error":f"Incorrectly formatted request: {e}"}), 400)
    
    if DEBUG:
        print(data)
        # cur.execute("SELECT * FROM Rovers")
        # for mac, nickname in cur:
        #     print(mac, nickname)
        # cur.execute("SELECT * FROM Diagnostics")
        # for mac, timestamp, battery, connection, sessionid in cur:
        #     print(mac, timestamp, battery, connection, sessionid, "THIS IS IN Diagnostics Table")
        cur.execute("SELECT * FROM ReplayInfo")
        for timestamp, xpos, ypos, whereat, orientation, tofleft, tofright, mac, SessionID in cur:
            print(timestamp, xpos, ypos, whereat, orientation, tofleft, tofright, mac, SessionID)
            app.logger.debug("TOFLEFT: "+str(tofleft)+" TOFRIGHT: "+str(tofright)+" YAW: "+str(orientation))
        # cur.execute("SELECT * FROM Sessions")
        # for mac, sessionId, SessionNickname in cur:
        #     print(mac, sessionId, SessionNickname)
        print(r.actions, "ACTIONS")
        print(resp)
    return make_response(jsonify(resp), 200)

# Give all rovers, active or inactive, to client
@app.route("/client/allrovers", methods=["GET"])
def allrovers():
    d = []
    disallowedMacs = []
    # remove timed out rovers
    for rover in rovers:
        if time()-rover.lastSeen > TIMEOUT:
            rovers.remove(rover)

    # add all active rovers
    for rover in rovers:
        temp = {}
        temp["MAC"] = rover.name
        disallowedMacs.append(rover.name)
        temp["nickname"] = rover.nickname
        temp["connected"] = True
        temp["sessionid"] = rover.sessionId
        d.append(temp)
    
    # add all inactive rovers from database
    t = ""
    for i in disallowedMacs:
        t+="MAC != "+str(i)+" AND "
    t = t[:-5]
    command = "SELECT * FROM Rovers WHERE "+t
    if command == "SELECT * FROM Rovers WHERE ":
        command = "SELECT * FROM Rovers" 
    try:
        cur.execute(command)
    except mariadb.Error as e:
        return make_response(jsonify({"error":f"Incorrectly formatted request: {e}"}), 400)
    for rover in cur:
        temp = {}
        temp["MAC"] = rover[0]
        temp["nickname"] = rover[1]
        temp["connected"] = False
        d.append(temp)
    
    if DEBUG:
        print(command)
        print(d)
        print(disallowedMacs)
    return make_response(jsonify(d), 200)

# get a specific session replay
@app.route("/client/replay", methods=["POST"])
def replay():
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
    if DEBUG:
        print(data)
        print(resp)
    return make_response(jsonify(resp))

# Get all sessions
@app.route("/client/sessions", methods=["GET"])
def sessions():
    cur.execute("SELECT * FROM Sessions ORDER BY SessionId DESC;")
    d = []

    for mac, sessionid, nickname in cur:
        temp = {}
        temp["sessionid"] = sessionid
        temp["MAC"] = mac
        temp["nickname"] = nickname
        d.append(temp)
    
    if DEBUG:
        print(d)
    return make_response(jsonify(d), 200)

# Get diagnostic data from given session
@app.route("/client/diagnostics", methods=["POST"])
def diagnostics():
    data = request.get_json()
    
    try:
        cur.execute("SELECT * FROM Diagnostics WHERE SessionID=? ORDER BY timestamp DESC;", (data["sessionid"],))
    except:
        return make_response(jsonify({"error":"Incorrectly formatted request: missing/invalid sessionid"}), 400)
    
    d = []
    for mac, timestamp, battery, connection, sessionid in cur:
        t = {"MAC":mac, "timestamp":timestamp, "battery":battery, "connection":connection}
        d.append(t)
    
    if DEBUG:
        print(data)
        print(d, "diagnostics")
    return make_response(jsonify(d), 200)

# Pause a given rover
@app.route("/client/pause", methods=["POST"])
def pause():
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
    
    if DEBUG:
        print("PAUSED")
        print(data)
    return make_response(jsonify({"success":"successfully paused rover"}), 200)

# Play a given rover
@app.route("/client/play", methods=["POST"])
def play():
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
    if DEBUG:
        print("PLAYED")
        print(data)
    return make_response(jsonify({"success":"successfully played rover"}), 200)

# Set a nickname for a session
@app.route("/client/sessionnickname", methods=["POST"])
def sessionNickname():
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
        tree[(node_x, node_y)] = [eval(x) for x in loads(children)]
    
    # assert that a tree is valid and formatted bidirectionally
    tree = dijkstra.assertValid(tree)

    # get tree predecessor graph
    P = dijkstra.dijkstra(tree, [float(start_x), float(start_y)])

    if P is None:
        return make_response(jsonify({"error":"Invalid starting point"}), 400)
    
    P = dijkstra.formatPredecessor(P)

    if DEBUG:
        print(tree, "shortestPath tree")
        print(P, "shortestPath predecessor")
    return make_response(jsonify(P), 200) # for testing

# emergency stops rover - must be disconnected and reconnected
@app.route("/client/estop", methods=["POST"])
def estop():
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

# turns on or off red LED
@app.route("/led_driver/red", methods=["POST"])
def led_driver_red():
    global isSpinning, spinTime
    data = request.get_json()
    
    if DEBUG:
        print(isSpinning, time()-spinTime)
    
    # logic to turn on led
    if isSpinning and time()-spinTime < TIMEOUT/3:
        return make_response(jsonify({"success":"received data", "switch":1}), 200)
    # logic to turn off led
    else:
        isSpinning = False
        return make_response(jsonify({"success":"received data", "switch":1}), 200)

@app.route("/led_driver/blue", methods=["POST"])
def led_driver_blue():
    global isSpinning, spinTime
    data = request.get_json()
    if DEBUG:
        print(isSpinning, time()-spinTime)
    if isSpinning and time()-spinTime < TIMEOUT/3:
        return make_response(jsonify({"success":"received data", "switch":1}), 200)
    else:
        isSpinning = False
        return make_response(jsonify({"success":"received data", "switch":1}), 200)

@app.route("/led_driver/yellow", methods=["POST"])
def led_driver_yellow():
    global isSpinning, spinTime
    data = request.get_json()
    if DEBUG:
        print(isSpinning, time()-spinTime)
    if isSpinning and time()-spinTime < TIMEOUT/3:
        return make_response(jsonify({"success":"received data", "switch":1}), 200)
    else:
        isSpinning = False
        return make_response(jsonify({"success":"received data", "switch":1}), 200)
    

#---------------------ERROR HANDLING------------------------#
@app.errorhandler(400)
def bad_request(e):
    return make_response(jsonify({"error":str(e)}), 400)

@app.errorhandler(404)
def not_found(e):
    return make_response(jsonify({"error":str(e)}), 404)

@app.errorhandler(500)
def internal_error(e):
    return make_response(jsonify({"error":str(e)}), 500)
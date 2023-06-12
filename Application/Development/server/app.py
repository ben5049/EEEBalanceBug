#!/bin/bash
from flask import Flask, jsonify, request, make_response
from flask_cors import CORS
import tremaux, dijkstra
import mariadb
from time import time
from json import loads

app = Flask(__name__)
cors = CORS(app, resources={r"/*":{"origins":"*"}})
TIMEOUT = 10
rovers = []
isSpinning = False
spinTime = time()
DEBUG = True
hostip = '54.209.183.94'
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

@app.route("/")
def hello():
    return jsonify({"hello":"world"})

# rover communication - works
@app.route("/rover", methods=["POST"])
def rover():
    data = request.get_json() # data has keys "diagnostics", "MAC", "nickname", "timestamp", "position", "whereat", "orientation", "branches", "beaconangles", "tofleft", "tofright"
    
    r = 0
    flag = True
    for rover in rovers:
        if rover.name == data["MAC"]:
            r = rover
            flag = False
            break
    if flag:
        r = tremaux.Rover(data["position"], data["whereat"], data["MAC"])
        r.pause = True
        r.nickname = data["nickname"]
        
        try:
            cur.execute("INSERT INTO Rovers (MAC, nickname) VALUES (? , ?)", (str(data["MAC"]), data["nickname"]))
        except mariadb.Error as e:
            cur.execute("SELECT * FROM Rovers WHERE MAC=?", (str(data["MAC"]),))
            flag = True
            for mac, nick in cur:
                print(mac, nick)
                if mac == data["MAC"]:
                    r.nickname = nick
                    flag = False
            if flag:
                return make_response(jsonify({"error":"Specified MAC does not exist"}), 400)                
        try:
            cur.execute("INSERT INTO Sessions (MAC,  SessionNickname) VALUES (?, ?)", (data["MAC"], r.startup+int(data["timestamp"])))
            cur.execute("SELECT MAX(SessionID) FROM Sessions WHERE MAC=?", (data["MAC"],))
        except mariadb.Error as e:
            return make_response(jsonify({"error":f"Incorrectly formatted request: {e}"}), 400)
        for x in cur:
            r.sessionId = x[0]
        
        rovers.append(r)
    
    r.lastSeen = time()
    resp = r.tremaux(data["position"], data["whereat"], data["branches"], data["beaconangles"], data["orientation"])
    resp = {"next_actions" : resp, "clear_queue":r.estop}
    if 1 in resp["next_actions"]:
        global isSpinning, spinTime
        isSpinning = True
        spinTime = time()
    
    # store positions and timestamp in database
    # also store tree in database if rover is done
    try:
        cur.execute("INSERT INTO ReplayInfo (timestamp, xpos, ypos, whereat, orientation, tofleft, tofright, MAC, SessionID) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)", (r.startup+int(data["timestamp"]), data["position"][0], data["position"][1], data["whereat"], data["orientation"], data["tofleft"], data["tofright"], data["MAC"], r.sessionId))
        cur.execute("INSERT INTO Diagnostics (MAC, timestamp, battery, connection, SessionID) VALUES (?, ?, ?, ?, ?)", (data["MAC"], r.startup+int(data["timestamp"]), data["diagnostics"]["battery"], data["diagnostics"]["connection"], r.sessionId))
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
        # cur.execute("SELECT * FROM Rovers")
        # for mac, nickname in cur:
        #     print(mac, nickname)
        cur.execute("SELECT * FROM Diagnostics")
        for mac, timestamp, battery, connection, sessionid in cur:
            print(mac, timestamp, battery, connection, sessionid, "THIS IS IN Diagnostics Table")
        # cur.execute("SELECT * FROM ReplayInfo")
        # for timestamp, xpos, ypos, whereat, orientation, tofleft, tofright, mac, SessionID in cur:
        #     print(timestamp, xpos, ypos, whereat, orientation, tofleft, tofright, mac, SessionID)
        # cur.execute("SELECT * FROM Sessions")
        # for mac, sessionId, SessionNickname in cur:
        #     print(mac, sessionId, SessionNickname)
        print(r.actions, "ACTIONS")
        print(resp)
    return make_response(jsonify(resp), 200)

# works
@app.route("/client/allrovers", methods=["GET"])
def allrovers():
    d = []
    disallowedMacs = []
    for rover in rovers:
        if time()-rover.lastSeen > TIMEOUT:
            rovers.remove(rover)
    
    for rover in rovers:
        temp = {}
        temp["MAC"] = rover.name
        disallowedMacs.append(rover.name)
        temp["nickname"] = rover.nickname
        temp["connected"] = True
        temp["sessionid"] = rover.sessionId
        d.append(temp)
    # get the rest of unconnected rovers from database
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

# works
@app.route("/client/replay", methods=["POST"])
def replay():
    data = request.get_json()
    try:
        sessionid = int(data["sessionid"])
    except:
        return make_response(jsonify({"error":"Incorrectly formatted request: missing sessionid"}), 400)
    try:
        cur.execute("SELECT * FROM ReplayInfo WHERE SessionID=?", (sessionid,))
    except mariadb.Error as e:
        return make_response(jsonify({"error":f"Incorrectly formatted request: {e}"}), 400)

    if cur is None:
        return make_response(jsonify({"error":"Session does not exist"}), 404)
    resp = {}
    for timestamp, xpos, ypos, whereat, orientation, tofleft, tofright, mac, sessionid in cur:
        resp[timestamp] = [xpos, ypos, whereat, orientation, tofleft, tofright]
    if DEBUG:
        print(data)
        print(resp)
    return make_response(jsonify(resp))

# works
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

# works
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

# works
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

# works
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

# works
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

# works
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

    for sid, node_x, node_y, children in cur:
        tree[(node_x, node_y)] = [eval(x) for x in loads(children)]
    
    tree = dijkstra.assertValid(tree)

    P = dijkstra.dijkstra(tree, [float(start_x), float(start_y)])

    if P is None:
        return make_response(jsonify({"error":"Invalid starting point"}), 400)
    P = dijkstra.formatPredecessor(P)

    if DEBUG:
        print(data)
        print(tree)
        print(P)
    return make_response(jsonify(P), 200) # for testing


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
    if DEBUG:
        print(data)
    return make_response(jsonify({"success":"estopped"}))

@app.route("/led_driver/red", methods=["POST"])
def led_driver_red():
    global isSpinning, spinTime
    data = request.get_json()
    print(data, "red")
    if isSpinning and time()-spinTime < TIMEOUT/3:
        return make_response(jsonify({"success":"received data", "switch":1}), 200)
    else:
        isSpinning = False
        return make_response(jsonify({"success":"received data", "switch":0}), 200)

@app.route("/led_driver/blue", methods=["POST"])
def led_driver_blue():
    global isSpinning, spinTime
    data = request.get_json()
    print(data, "blue")
    if isSpinning and time()-spinTime < TIMEOUT/3:
        return make_response(jsonify({"success":"received data", "switch":1}), 200)
    else:
        isSpinning = False
        return make_response(jsonify({"success":"received data", "switch":0}), 200)

@app.route("/led_driver/yellow", methods=["POST"])
def led_driver_yellow():
    global isSpinning, spinTime
    data = request.get_json()
    print(data, "yellow")
    if isSpinning and time()-spinTime < TIMEOUT/3:
        return make_response(jsonify({"success":"received data", "switch":1}), 200)
    else:
        isSpinning = False
        return make_response(jsonify({"success":"received data", "switch":0}), 200)
    

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
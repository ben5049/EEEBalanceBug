from flask import Flask, jsonify, request, make_response
import tremaux, dijkstra
import mariadb
# TODO: error handling

app = Flask(__name__)

hostip = '3.88.27.3'
# database set to run on port 3306, flask server set to run on port 5000 (when deploying, not developing)
try:
    conn = mariadb.connect(
        user='bb',
        password='',
        host=hostip,
        port=3306,
        database='BalanceBug'
    )
except mariadb.Error as e:
    print(f"Error connecting to MariaDB platform: {e}")

cur = conn.cursor()

rovers = []

@app.route("/")
def hello():
    return jsonify({"hello":"world"})

# rover communication - works
@app.route("/rover", methods=["POST"])
def rover():
    data = request.get_json() # data has keys "diagnostics", "MAC", "nickname", ""timestamp", "position", "whereat", "orientation", "branches", "beaconangles", "tofleft", "tofright"
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
        try:
            cur.execute("INSERT INTO Rovers (MAC, nickname) VALUES (? , ?)", (data["MAC"], data["nickname"]))
            cur.execute("INSERT INTO Sessions (MAC,  SessionNickname) VALUES (?, ?)", (data["MAC"], data["timestamp"]))
            cur.execute("SELECT MAX(SessionID) FROM Sessions WHERE MAC=?", (data["MAC"],))
        except mariadb.Error as e:
            return make_response(jsonify({f"error":"Incorrectly formatted request: {e}"}), 400)
        for x in cur:
            r.sessionId = x[0]
        
        rovers.append(r)

    resp = r.tremaux(data["position"], data["whereat"], data["branches"], data["beaconangles"])
    resp = {"next_actions" : resp}
    
    # store positions and timestamp in database
    try:
        cur.execute("INSERT INTO ReplayInfo (timestamp, xpos, ypos, whereat, orientation, tofleft, tofright, MAC, SessionID) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)", (data["timestamp"], data["position"][0], data["position"][1], data["whereat"], data["orientation"], data["tofleft"], data["tofright"], data["MAC"], r.sessionId))
        cur.execute("INSERT INTO Diagnostics (MAC, timestamp, battery, CPU, connection) VALUES (?, ?, ?, ?, ?)", (data["MAC"], data["timestamp"], data["diagnostics"]["battery"], data["diagnostics"]["CPU"], data["diagnostics"]["connection"]))
    except mariadb.Error as e:
        return make_response(jsonify({f"error":"Incorrectly formatted request: {e}"}), 400)
    # cur.execute("SELECT * FROM Rovers")
    # for mac, nickname in cur:
    #     print(mac, nickname)
    # cur.execute("SELECT * FROM Diagnostics")
    # for mac, timestamp, battery, cpu, connection in cur:
    #     print(mac, timestamp, battery, cpu, connection)
    cur.execute("SELECT * FROM ReplayInfo")
    for timestamp, xpos, ypos, whereat, orientation, tofleft, tofright, mac, SessionID in cur:
        print(timestamp, xpos, ypos, whereat, orientation, tofleft, tofright, mac, SessionID)
    # cur.execute("SELECT * FROM Sessions")
    # for mac, sessionId, SessionNickname in cur:
    #     print(mac, sessionId, sessionNickname)

    return make_response(jsonify(resp), 200)

# works
@app.route("/client/allrovers", methods=["GET"])
def allrovers():
    d = []
    disallowedMacs = []
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
        t+="MAC <> "+str(i)+" AND "
    t = t[:-5]
    command = "SELECT * FROM Rovers WHERE "+t
    cur.execute(command)
    for rover in cur:
        temp = {}
        temp["MAC"] = rover[0]
        temp["nickname"] = rover[1]
        temp["connected"] = False
        temp["sessionid"] = None
        d.append(temp)
    return make_response(jsonify(d), 200)

# works
@app.route("/client/replay", methods=["GET"])
def replay():
    data = request.get_json()
    try:
        sessionid = int(data["sessionid"])
    except:
        return make_response(jsonify({"error":"Incorrectly formatted request: missing sessionid"}), 400)
    
    cur.execute("SELECT * FROM ReplayInfo")
    cur.execute("SELECT * FROM ReplayInfo WHERE SessionID=?", (sessionid,))
    if cur is None or len(cur)==0:
        return make_response(jsonify({"error":"Session does not exist"}), 404)
    resp = {}
    for timestamp, xpos, ypos, whereat, orientation, tofleft, tofright, mac, sessionid in cur:
        resp[timestamp] = [xpos, ypos, whereat, orientation, tofleft, tofright]
    return make_response(jsonify(resp))

# works
@app.route("/client/sessions", methods=["GET"])
def sessions():
    cur.execute("SELECT * FROM Sessions ORDER BY SessionId DESC;")
    d = {}
    for mac, sessionid, nickname in cur:
        d[sessionid] = [mac, nickname]
    return make_response(jsonify(d), 200)

# works
@app.route("/client/diagnostics", methods=["GET"])
def diagnostics():
    data = request.get_json()
    try:
        cur.execute("SELECT * FROM Diagnostics WHERE MAC=? ORDER BY timestamp DESC LIMIT 1;", (data["MAC"],))
    except:
        return make_response(jsonify({"error":"Incorrectly formatted request: missing MAC address"}), 400)
    d = {}
    for mac, timestamp, battery, cpu, connection in cur:
        d = {"MAC":mac, "timestamp":timestamp, "battery":battery, "CPU":cpu, "connection":connection}
    return make_response(jsonify(d), 200)

# works
@app.route("/client/pause", methods=["POST"])
def pause():
    data = request.get_json()
    try:
        mac = data["MAC"]
    except:
        return make_response(jsonify({"error":"Incorrectly formatted request: {e}"}), 400)
    flag = True
    for rover in rovers:
        if rover.name == mac:
            rover.pause = True
            flag = False

    if flag:
        return make_response(jsonify({"error":"Selected rover does not exist, or is not currently connected"}), 400)
    
    return make_response(jsonify({"success":"successfully paused rover"}), 200)

# works
@app.route("/client/play", methods=["POST"])
def play():
    data = request.get_json()
    try:
        mac = data["MAC"]
    except:
        return make_response(jsonify({"error":"Incorrectly formatted request: missing MAC"}), 400)
    flag = True
    for rover in rovers:
        if rover.name == mac:
            rover.pause = False
            flag = False
    if flag:
        return make_response(jsonify({"error":"Selected rover does not exist, or is not currently connected"}), 400)
    
    return make_response(jsonify({"success":"successfully played rover"}), 200)

# works
@app.route("/client/setsessionnickname", methods=["POST"])
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
        return make_response(jsonify({f"error":"Incorrectly formatted request: {e}"}), 400)
    return make_response(jsonify({"success":"successfully changed session nickname"}), 200)

# works
@app.route("/client/addnickname", methods=["POST"])
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
        return make_response(jsonify({f"error":"Incorrectly formatted request: {e}"}), 400)

    return make_response(jsonify({"success":"successfully changed rover nickname"}), 200)

@app.route("/client/shortestpath", methods=["POST"])
def findShortestPath():
    data = request.get_json()
    try:
        mac = data["MAC"]
        start = data["start"]
        end = data["end"]
    except:
        return make_response(jsonify({"error":"Incorrectly formatted request: missing mac, start, or end"}), 400)
    tree = 0
    for rover in rovers:
        if rover.name == mac:
            tree = rover.tree
    P = dijkstra.dijkstra(tree, start)
    betterP = {}
    for key, value in P:
        betterP[key.position] = betterP[value.position]

    return make_response(jsonify(betterP), 200)


    



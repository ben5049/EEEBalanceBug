from flask import Flask, jsonify, request, make_response
import tremaux, dijkstra
import mariadb
# TODO: error handling

app = Flask(__name__)

hostip = '3.88.27.3'

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
        cur.execute("INSERT INTO Rovers (MAC, nickname) VALUES (? , ?)", (data["MAC"], data["nickname"]))
        cur.execute("INSERT INTO Sessions (MAC,  SessionNickname) VALUES (?, ?)", (data["MAC"], data["timestamp"]))
        cur.execute("SELECT MAX(SessionID) FROM Sessions WHERE MAC=?", (data["MAC"],))
        for x in cur:
            r.sessionId = x[0]
        
        rovers.append(r)

    resp = r.tremaux(data["position"], data["whereat"], data["branches"], data["beaconangles"])
    resp = {"next_actions" : resp}
    print(data)
    # store positions and timestamp in database
    cur.execute("INSERT INTO ReplayInfo (timestamp, xpos, ypos, whereat, orientation, tofleft, tofright, MAC, SessionID) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)", (data["timestamp"], data["position"][0], data["position"][1], data["whereat"], data["orientation"], data["tofleft"], data["tofright"], data["MAC"], r.sessionId))
    cur.execute("INSERT INTO Diagnostics (MAC, timestamp, battery, CPU, connection) VALUES (?, ?, ?, ?, ?)", (data["MAC"], data["timestamp"], data["diagnostics"]["battery"], data["diagnostics"]["CPU"], data["diagnostics"]["connection"]))

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
    sessionid = int(data["sessionid"])
    cur.execute("SELECT * FROM ReplayInfo")
    for timestamp, xpos, ypos, whereat, orientation, tofleft, tofright, mac, SessionID in cur:
        print(timestamp, xpos, ypos, whereat, orientation, tofleft, tofright, mac, SessionID)
    print(sessionid)
    cur.execute("SELECT * FROM ReplayInfo WHERE SessionID=?", (sessionid,))
    if cur is None:
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
    cur.execute("SELECT * FROM Diagnostics WHERE MAC=? ORDER BY timestamp DESC LIMIT 1;", (data["MAC"],))
    d = {}
    for mac, timestamp, battery, cpu, connection in cur:
        d = {"MAC":mac, "timestamp":timestamp, "battery":battery, "CPU":cpu, "connection":connection}
    print(d)
    return make_response(jsonify(d), 200)

@app.route("/client/visualisations", methods=["GET"])
def visualisations():
    return
# works
@app.route("/client/pause", methods=["POST"])
def pause():
    data = request.get_json()
    mac = data["MAC"]
    for rover in rovers:
        if rover.name == mac:
            rover.pause = True

    return make_response(jsonify({"success":"successfully paused rover"}), 200)

# works
@app.route("/client/play", methods=["POST"])
def play():
    data = request.get_json()
    mac = data["MAC"]
    for rover in rovers:
        if rover.name == mac:
            rover.pause = False
    return make_response(jsonify({"success":"successfully played rover"}), 200)

# works
@app.route("/client/setsessionnickname", methods=["POST"])
def sessionNickname():
    data = request.get_json()
    session = data["sessionid"]
    nick = data["sessionNick"]
    cur.execute("UPDATE Sessions SET SessionNickname=? WHERE SessionId=?", (nick, session,))
    return make_response(jsonify({"success":"successfully updated session nickname"}), 200)

# works
@app.route("/client/addnickname", methods=["POST"])
def addnickname():
    data = request.get_json()
    mac = data["MAC"]
    nick = data["nickname"]
    for rover in rovers:
        if rover.name == mac:
            rover.nickname = nick

    cur.execute("UPDATE Rovers SET nickname=? WHERE MAC=?", (nick, mac,))
    return make_response(jsonify({"success":"successfully changed rover/session nickname"}), 200)

@app.route("/client/shortestpath", methods=["POST"])
def findShortestPath():
    data = request.get_json()
    mac = data["MAC"]
    start = data["start"]
    end = data["end"]
    tree = 0
    for rover in rovers:
        if rover.name == mac:
            tree = rover.tree
    P = dijkstra.dijkstra(tree, start)
    betterP = {}
    for key, value in P:
        betterP[key.position] = betterP[value.position]

    return make_response(jsonify(betterP), 200)


    



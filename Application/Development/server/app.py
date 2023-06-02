from flask import Flask, jsonify, request, make_response
import tremaux, dijkstra
import mariadb
# TODO: database stuff

app = Flask(__name__)

hostip = '3.86.226.249'

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

# rover communication
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
        r.sessionId = cur[0]
        
        rovers.append(r)

    resp = r.tremaux(data["position"], data["whereat"], data["branches"], data["beaconangles"])
    resp = {"next_actions" : resp}

    # store positions and timestamp in database
    cur.execute("INSERT INTO Diagnostics (MAC, timestamp, battery, CPU, connection) VALUES (?, ?, ?, ?, ?)", (data["MAC"], data["timestamp"], data["diagnostics"]["battery"], data["diagnostics"]["CPU"], data["diagnostics"]["connection"]))
    cur.execute("INSERT INTO ReplayInfo (timestamp, xpos, ypos, whereat, orientation, tofleft, tofright, MAC, SessionID) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)", (data["timestamp"], data["position"][0], data["position"][1], data["whereat"], data["orientation"], data["tofleft"], data["tofright"], data["MAC"], r.sessionId))
    
    return make_response(jsonify(resp), 200)

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
        temp["sessionid"] = rover.sessionid
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


@app.route("/client/replay", methods=["GET"])
def replay():
    data = request.get_json()
    sessionid = data["sessionid"]
    info = cur.execute("SELECT * FROM ReplayInfo WHERE SessionID=?", (sessionid,))
    resp = {}
    for timestamp, xpos, ypos, whereat, orientation, tofleft, tofright, mac, sessionid in info:
        resp[timestamp] = [xpos, ypos, whereat, orientation, tofleft, tofright]
    return make_response(jsonify(resp))

@app.route("/client/sessions", methods=["GET"])
def sessions():
    cur.execute("SELECT * FROM SESSIONS ORDER BY SessionId DESC;")
    d = {}
    for mac, sessionid, nickname in cur:
        d[sessionid] = [mac, nickname]
    return make_response(jsonify(d), 200)

@app.route("/client/diagnostics", methods=["GET"])
def diagnostics():
    data = request.get_json()
    instr = "SELECT * FROM Diagnostics WHERE MAC=? ORDER BY timestamp DESC LIMIT 1;"
    cur.execute(instr, (data["MAC"]))
    d = {"MAC":cur[0], "timestamp":cur[1], "battery":cur[2], "CPU":cur[3], "connection":cur[4]}
    return make_response(jsonify(d))

@app.route("/client/visualisations", methods=["GET"])
def visualisations():
    return

@app.route("/client/pause", methods=["POST"])
def pause():
    data = request.get_json()
    mac = data["MAC"]
    for rover in rovers:
        if rover.name == mac:
            rover.pause = True

    return make_response(jsonify({"success":"successfully paused rover"}), 200)

@app.route("/client/play", methods=["POST"])
def play():
    data = request.get_json()
    mac = data["MAC"]
    for rover in rovers:
        if rover.name == mac:
            rover.pause = False
    return make_response(jsonify({"success":"successfully played rover"}), 200)

@app.route("/client/setsessionnickname", methods=["POST"])
def sessionNickname():
    data = request.get_json()
    session = data["sessionid"]
    nick = data["sessionNick"]
    cur.execute("UPDATE Sessions SET SessionNickname="+nick+" WHERE SessionId=?", (session,))
    return make_response(jsonify({"success":"successfully updated session nickname"}), 200)

@app.route("/client/addnickname", methods=["POST"])
def addnickname():
    data = request.get_json()
    mac = data["MAC"]
    nick = data["nickname"]
    for rover in rovers:
        if rover.name == mac:
            rover.nickname = nick

    cur.execute("UPDATE Rovers SET nickname="+nick+" WHERE MAC=?", (mac,))
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


    



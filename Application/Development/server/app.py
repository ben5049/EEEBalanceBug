from flask import Flask, jsonify, request, make_response
import tremaux, dijkstra
# TODO: database stuff
app = Flask(__name__)

rovers = []

@app.route("/")
def hello():
    return jsonify({"hello":"world"})

# rover communication
@app.route("/rover", methods=["GET"])
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
        rovers.append(r)
    
    resp = r.tremaux(data["position"], data["whereat"], data["branches"], data["beaconangles"])
    resp = {"next_actions" : resp}

    # store positions and timestamp in database - figure out how

    return make_response(jsonify(resp), 200)

@app.route("/client/allrovers", methods=["GET"])
def allrovers():
    d = []
    for rover in rovers:
        temp = {}
        temp["MAC"] = rover.name
        temp["nickname"] = rover.nickname
        temp["connected"] = True
        d.append(temp)
    # get the rest of unconnected rovers from database

    return make_response(jsonify(d), 200)

@app.route("/client/allreplays", methods=["GET"])
def allreplays():
    return

@app.route("/client/replay", methods=["GET"])
def replay():
    return

@app.route("/client/diagnostics", methods=["GET"])
def diagnostics():
    return

@app.route("/client/visualisations", methods=["GET"])
def visualisations():
    return

@app.route("client/pause", methods=["POST"])
def pause():
    data = request.get_json()
    mac = data["MAC"]
    for rover in rovers:
        if rover.name == mac:
            rover.pause = True

    return make_response(jsonify({"success":"successfully paused rover"}), 200)

@app.route("client/play", methods=["POST"])
def play():
    data = request.get_json()
    mac = data["MAC"]
    for rover in rovers:
        if rover.name == mac:
            rover.pause = False
    return make_response(jsonify({"success":"successfully played rover"}), 200)


@app.route("client/addnickname", methods=["POST"])
def addnickname():
    data = request.get_json()
    mac = data["MAC"]
    nick = data["nickname"]
    for rover in rovers:
        if rover.name == mac:
            rover.nickname = nick
    return make_response(jsonify({"success":"successfully changed rover nickname"}), 200)

@app.route("client/shortestpath", methods=["POST"])
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


    



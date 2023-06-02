const express = require("express");
const cors = require('cors');
const PORT = process.env.PORT || 3001;
const app = express();

var mysql = require('mysql');
var con = mysql.createConnection(
{
 host: "18.134.133.102", // localhost for AWS
 user: "username",
 password: "usrpwd",
 database: "Persondb"
});
con.connect(function(err) {
 if (err) throw err;
 console.log("Successfully connected to the database...\n");
});



app.use(cors({
            origin: '*'
            }));
app.use(cors({
            methods: ['GET','POST','DELETE','UPDATE','PUT','PATCH']
            }));
app.use(express.json());


// test endpoints (delete)

app.get("/tableData33", (req, res) => {
    res.json(
    {
    "tableData33":[
        ['Ed', 15 + Math.floor(Math.random() * 35), 'Male'],
        ['Mia', 15 + Math.floor(Math.random() * 35), 'Female'],
        ['Max', 15 + Math.floor(Math.random() * 35), 'Male']
    ]
    })
});

app.get("/personQuery", (req, res) => {
    con.query("SELECT * FROM Persons", function (err, result, fields) {
        if (err) throw err;
        res.json(result[0])
    });
});

app.get("/pollServer", (req, res) => {
    var d = new Date();
    const json_res = {
        "time" : d.toTimeString()
    };
    res.send(json_res);
}); 

app.post('/TestPOST', (req, res) => {
    // Access the request body using req.body
    console.log(req.body);
    // Example: Return a response with the received data
    res.json({ message: 'Received POST request', data: req.body });
});

/////////////////////
// Rover Endpoints //
/////////////////////

app.post('/RoverPOST', (req, res) => { // Get data from Rover
    // RECEIVES MAC, Timestamp, data
    console.log('RoverPOST received: ' + req.body);
    res.json({ message: 'Received POST request', data: req.body });
    // UPDATE ROVER COMMAND FROM SCRIPT
    // UPDATE VISUALISER ARGS
    // WRITE TO DATABASE
});

app.post('/RoverCommand', (req, res) => { // Get data from Rover
    // RECEIVES MAC, Timestamp
    // DISPLAY ROVER COMMAND FROM SCRIPT (some variable)
    console.log('RoverCommand request received from: ' + req.body);
    res.json({ "command": 'FORWARD?'});
});

//////////////////////
// Laptop Endpoints //
//////////////////////

// Menu
app.get("/MenuData", (req, res) => {
    res.send({"data" : "DATA GIVEN HERE", "replays" : "[[MAC, Timestamp, MapImageName], ...]"});
});

app.get("/MenuRoverImage", (req, res) => {
    // SEND Generic Image of rover
    res.send({"data" : "DATA GIVEN HERE", "replays" : "[[MAC, Timestamp, MapImageName], ...]"});
});

// Rover Diagnostics
app.get("/LaptopLiveData", (req, res) => { // TODO: change to post
    res.send({"data" : "DATA GIVEN HERE"});
});

// Live Map Update
app.post("/LaptopLiveMap", (req, res) => {
    console.log('Client request for rover: ' + req.body);
    res.json({timestamp: "LAST UPDATE TIME FROM SCRIPT", data : "{orientation : 'N', TOF-L : 'X', TOF-R : 'G'}"});
}); 

// Start Mapping (start script)
app.post('/LaptopStartMapCommand', (req, res) => { // Get data from Rover
    // RECEIVES MAC, Timestamp
    console.log('RoverPOST received: ' + req.body);
    res.json({ message: 'Received POST request', data: req.body });
});

app.post('/LaptopReplay', (req, res) => { // Get data from Rover
    // RECEIVES MAC, Timestamp
    console.log('RoverPOST received: ' + req.body);
    // RETURNS Table for given MAC & Timestamp 
    res.json({ message: 'Received POST request', data: req.body });
});






app.listen(PORT, () => {
 console.log(`Server listening on ${PORT}`);
});
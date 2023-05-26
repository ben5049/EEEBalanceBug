const express = require("express");
const cors = require('cors');
const PORT = process.env.PORT || 3001;
const app = express();

var mysql = require('mysql');
var con = mysql.createConnection(
{
 host: "35.178.189.252", // UPDATE FOR NEW EC2 INSTANCE (localhost for on AWS?)
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

app.get("/pollServer", (req, res) => {
    var d = new Date();
    const json_res = {
    "time" : d.toTimeString()
    };
    res.send(json_res);
    }); 


app.listen(PORT, () => {
 console.log(`Server listening on ${PORT}`);
});
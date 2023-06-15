# ask user for server ip
SERVER=$1
# regex for ip address
SEDTEXT="s/[0-9]*\.[0-9]*\.[0-9]*\.[0-9]*/"
# change server ip for database usage and client & rover API requests  
SEDTEXT2="/ ./server/app.py"
sudo sed -i ${SEDTEXT}${SERVER}${SEDTEXT2}
SEDTEXT3="/ ./client/src/App.js"
sudo sed -i ${SEDTEXT}${SERVER}${SEDTEXT3}
SEDTEXT4="/ ../../Microcontroller_code/main/TaskServerCommunication.cpp"
sudo sed -i ${SEDTEXT}${SERVER}${SEDTEXT4}
# push changed files to server
git add ./server/app.py
git add ./client/src/App.js
git add ../../Microcontroller_code/main/TaskServerCommunication.cpp
git commit -m "IP Update"
git push

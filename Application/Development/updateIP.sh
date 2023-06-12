SERVER = $1
SEDTEXT="s/[0-9]*\.[0-9]*\.[0-9]*\.[0-9]*/"
SEDTEXT2="/ ./server/app.py"
sudo sed -i ${SEDTEXT}${SERVER}${SEDTEXT2}
SEDTEXT3="/ ./client/src/App.js"
sudo sed -i ${SEDTEXT}${SERVER}${SEDTEXT3}
git add ./server/app.py
git add ./client/src/App.js
git commit -m "IP Update"
git push

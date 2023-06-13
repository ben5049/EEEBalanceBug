#!/bin/sh
DEBUG=false
# ask user for server ip: "1.1.1.1"
SERVER=$1
# regex describing ip address format
SEDTEXT="s/[0-9]*\.[0-9]*\.[0-9]*\.[0-9]*/"
# regex describing file to change
SEDTEXT2="/ /etc/nginx/conf.d/react.conf"
# pull any changes and update nginx service
sudo git pull https://aranyagupta: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@github.com/ben5049/EEEBalanceBug
sudo sed -i ${SEDTEXT}${SERVER}${SEDTEXT2}
# build with new ip address and copy build to necessary place
cd client
npm run build
sudo cp -rf /home/ubuntu/EEEBalanceBug/Application/Development/client/build/* /var/www/html/react
cd ..
# reload the daemon and enable flask + nginx
sudo systemctl daemon-reload
sudo systemctl start flask
sudo systemctl enable flask
sudo systemctl restart nginx
if ($DEBUG)
then
    sudo systemctl status flask.service
fi
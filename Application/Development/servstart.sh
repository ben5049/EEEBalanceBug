#!/bin/sh
DEBUG=false
SERVER=$1
SEDTEXT="s/[0-9]*\.[0-9]*\.[0-9]*\.[0-9]*/"
SEDTEXT2="/ /etc/nginx/conf.d/react.conf"
sudo git pull https://aranyagupta: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@github.com/ben5049/EEEBalanceBug
sudo sed -i ${SEDTEXT}${SERVER}${SEDTEXT2}
cd client
npm run build
sudo cp -rf /home/ubuntu/EEEBalanceBug/Application/Development/client/build/* /var/www/html/react
cd ..
sudo systemctl daemon-reload
sudo systemctl start flask
sudo systemctl enable flask
sudo systemctl restart nginx
if ($DEBUG)
then
    sudo systemctl status flask.service
fi
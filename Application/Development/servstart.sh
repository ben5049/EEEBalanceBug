#!/bin/sh
DEBUG=false 
sudo git pull https://aranyagupta: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@github.com/ben5049/EEEBalanceBug
cd client
npm run build
sudo cp -rf build /var/www/react/html
sudo cp -rf public /var/www/react/html
sudo cp -rf src /var/www/react/html
sudo cp -rf package-lock.json /var/www/react/html
sudo cp -rf package.json /var/www/react/html
cd ..
sudo systemctl daemon-reload
sudo systemctl start flask
sudo systemctl enable flask
if ($DEBUG)
then
    sudo systemctl status flask.service
fi
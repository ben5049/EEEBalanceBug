#!/bin/sh
DEBUG=false 
sudo git pull https://aranyagupta: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@github.com/ben5049/EEEBalanceBug
cd client
npm run build
sudo cp -f build /var/www/html
sudo cp -f public /var/www/html
sudo cp -f src /var/www/html
sudo cp -f package-lock.json /var/www/html
sudo cp -f package.json /var/www/html
cd ..
sudo systemctl daemon-reload
sudo systemctl start flask
sudo systemctl enable flask
if ($DEBUG)
then
    sudo systemctl status flask.service
fi
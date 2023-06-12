#!/bin/sh
DEBUG=false 
sudo git pull https://aranyagupta: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@github.com/ben5049/EEEBalanceBug
cd client
npm run build
mv -rf build /var/www/html
mv -rf public /var/www/html
mv -rf src /var/www/html
mv -rf package-lock.json /var/www/html
mv -rf package.json /var/www/html
sudo systemctl daemon-reload
sudo systemctl start flask
sudo systemctl enable flask
if ($DEBUG)
then
    sudo systemctl status flask.service
fi
#!/bin/sh
DEBUG=false
# ask user for server ip: "1.1.1.1"
# pull any changes
sudo git pull https://aranyagupta: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@github.com/ben5049/EEEBalanceBug
# reload the daemon and enable flask + nginx
sudo systemctl daemon-reload
sudo systemctl start flask
sudo systemctl enable flask
sudo systemctl restart nginx
if ($DEBUG)
then
    sudo systemctl status flask.service
fi
#!/bin/sh
DEBUG=false 
sudo git pull https://aranyagupta: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@github.com/ben5049/EEEBalanceBug
sudo systemctl daemon-reload
sudo systemctl start flask
sudo systemctl enable flask
if ($DEBUG)
then
    sudo systemctl status flask.service
fi
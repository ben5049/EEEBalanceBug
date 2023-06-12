#!/bin/sh
DEBUG=false 

sudo systemctl daemon-reload
sudo systemctl start flask
sudo systemctl enable flask
if ($DEBUG)
then
    sudo systemctl status flask.service
fi
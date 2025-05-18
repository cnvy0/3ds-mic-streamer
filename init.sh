#!/bin/bash

sh createvirtualmic.sh
source ./venv/bin/activate
echo "CTRL + C to stop server"
echo "Virtual PulseAudio device will persist until reboot."
echo "--------------------------------------------------"
python3 server.py

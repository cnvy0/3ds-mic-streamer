# 3ds-mic-streamer

stream the input of a nintendo 3ds/2ds console's horrible microphone over udp to a linux pc

## how 2 use client:
- have modded 2ds/3ds  
- load the app from ur device 

## how 2 use server:
```bash
# get copy of this repo
# create a python venv
python -m venv venv
# set source to venv
source venv/bin/activate
# install python packages
pip install -r requirements.txt
# run init
./init.sh
# (to run the server you'll just have to run init.sh every time)
```

## how 2 compile 3ds app:
- install devkitpro
- get copy of this repo
- run ``make``

## notice
i will not be working on this project anymore so please dont expect changes or response to issues, unless it stops working on my machine of course  
i dont think that will change man im busy  
also i only decided to make this as my laptop's internal mic isn't detected, and i thought it would be funny to use the mic of my n2ds


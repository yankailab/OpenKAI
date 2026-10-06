# Config

## CAN transport streams

See [jsonCfg/_CAN.json](../jsonCfg/_CAN.json) for `_USR_CANET` and `_SocketCAN` examples. Both modules inherit `_ModuleBase` and use `CANframeStreamIn` for outgoing frames and `CANframeStreamOut` for received frames. Declare each stream with `"type": "dataObject"` and `"class": "CANframeStream"`.

`thread.FPS` controls sending; `threadR.FPS` controls receiving and defaults to the send rate when omitted. `_USR_CANET` encodes frames into `BytePacketStreamOut` and decodes `BytePacketStreamIn`. Configure both links for each enabled direction: `CANframeStreamIn` with `BytePacketStreamOut`, or `BytePacketStreamIn` with `CANframeStreamOut`. `_SocketCAN` uses `ifName` (default `can0`) and supports either or both frame streams.

Producers append `CAN_FRAME` values with `CANframeStream::add()`, supplying an increasing, nonzero `m_tStamp` on each frame. Transport workers call the parameterless `sendFrame()` and `readFrame()` methods; received frames get timestamps from the transport. These transports support classical CAN payloads of up to eight bytes, including extended IDs and RTR flags.

## Add user to dialout
```bash
sudo adduser $USER dialout
#sudo gpasswd --add ${USER} dialout
sudo reboot now
#sudo chmod 666 /dev/ttyUSB0
```

## When using multiple USB-Serial converter
```bash
sudo apt remove brltty
```

## HiK thermal camera config
add 99-hik.rules to /etc/udev/rules.d/
```bash
sudo nano /etc/udev/rules.d/99-hik.rules
SUBSYSTEM=="usb", ATTRS{idVendor}=="2bdf", MODE:="0666"
```

## Auto start using crontab
```bash
crontab -e
@reboot /home/lab/start.sh
```

## Auto startup using rc.local
Copy startup sh into home
```bash
sudo chmod a+x $HOME/ok.sh
```

For startup on X, insert sleep before running OK to wait for X to be ready
```bash
sleep 5
./ok.sh
```

Auto start up on boot
```bash
sudo rm /etc/rc.local
set +H
sudo sh -c "echo '#!/bin/sh\n/home/lab/ok.sh\nexit 0\n' >> /etc/rc.local"
set -H
sudo chmod a+x /etc/rc.local
```

# Screen
## Disable wayland in Ubuntu
```bash
sudo nano /etc/gdm3/custom.conf
#WaylandEnable=false
```

## Screen and touch screen input rotate
```bash
export DISPLAY=:0.0
xrandr -o left
xinput list
xinput set-prop 'GXTP7386:00 27C6:0113' 'Coordinate Transformation Matrix' 0 -1 1 1 0 0 0 0 1
```

## Block unity touch gestures
```bash
sudo apt install dconf-editor
dconf-editor
com > canonical > unity > gestures
```

## Block on screen keyboard
```bash
sudo apt install gnome-tweaks
sudo apt install gnome-shell-extensions
https://extensions.gnome.org/extension/3222/block-caribou-36/
```

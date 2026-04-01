# UM980(Base) + UM982(Rover) for RTK heading

## UM980 as base
com1:TXD/RXD, com2:TX2/RX2, com3: USB

Open COM port on UPrecise
```bash
freset
version
unlog
config com1 115200
config com2 115200
config com3 115200

mode base time 120 2.5
gngga com3 1
gnrmc com3 1
gngsa com3 1
gpgst com3 1
gngsv com3 1

rtcm1006 com1 10
rtcm1033 com1 10
rtcm1074 com1 1
rtcm1084 com1 1
rtcm1094 com1 1
rtcm1114 com1 1
rtcm1124 com1 1
rtcm1230 com1 10

saveconfig
```

## UM982 as rover
```bash
freset
version
unlog
config com1 115200
config com2 230400
config com3 115200

#config signalgroup 3 6   # recommended: master group 3, slave group 6 on UM982
#config pvtalg multi

// positions
mode rover uav
gpgga com2 0.2
gpgst com2 0.2
gprmc com2 0.2
gpgsa com2 0.2
gngga com2 0.2
gnrmc com2 0.2
gngsa com2 0.2
gngsv com2 0.2
agrica com2 0.2
uniheadinga com2 0.2
gpths com2 0.2
saveconfig
```

## Ardupilot settings
```bash
SERIAL3_PROTOCOL = 5
SERIAL3_BAUD = 230
GPS1_TYPE = 25      # Unicore NMEA, 24 for single antenna
GPS1_RATE_MS = 200  # 5Hz
GPS2_TYPE = 0
AHRS_EKF_TYPE = 3
EKF3_ENABLE = 1
EK3_SRC1_YAW = 2    # GPS, or 3:GPS with fallback to compass

# optional, disable compasses
COMPASS_USE = 0
COMPASS_USE2 = 0
COMPASS_USE3 = 0
```

## Antenna positioning
Master = left side,
Slave = right side.
On Ardupilot set Master in relation to Slave antenna in local NED coordinate:

```bash
GPS1_MB_TYPE = 1
GPS1_MB_OFS_X = N   # Forward positive
GPS1_MB_OFS_Y = E   # Right positive
GPS1_MB_OFS_Z = D   # Downward positive

#(Optional)Master antenna relative to the rover CG
GPS1_POS_X
GPS1_POS_Y
GPS1_POS_Z
```

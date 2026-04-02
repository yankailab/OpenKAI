# Build RTKLIB

```bash
git clone https://github.com/tomojitakasu/RTKLIB.git --branch rtklib_2.4.3
cd RTKLIB/app/consapp/str2str/gcc
make
```

```bash
./str2str -in ntrip://USER:PASSWORD@CORS_HOST:2101/MOUNTPOINT -out ./corr.rtcm3
```

```bash
./str2str -in  ntrip://USER:PASSWORD@CASTER_HOST:2101/MOUNTPOINT -out tcpsvr://:9001
```

## >2.4.3 b34
```bash
./str2str -in  ntrip://USER:PASSWORD@CASTER_HOST:2101/MOUNTPOINT -out udpsvr://:9000#rtcm3
```

```bash
./str2str -in  ntrip://USER:PASSWORD@CASTER_HOST:2101/MOUNTPOINT -out udpcli://127.0.0.1:9000#rtcm3
```


Sample RTCM received
```bash
msgID: 1077, nB=290, nRecv=19, tIntSec=0.86, bTout=0
msgID: 1087, nB=191, nRecv=18, tIntSec=1.13, bTout=0
msgID: 1097, nB=226, nRecv=18, tIntSec=1.13, bTout=0
msgID: 1117, nB=117, nRecv=18, tIntSec=1.13, bTout=0
msgID: 1127, nB=186, nRecv=18, tIntSec=1.13, bTout=0

msgID: 1006, nB=27, nRecv=3, tIntSec=5.08, bTout=0
msgID: 1008, nB=12, nRecv=3, tIntSec=5.08, bTout=0

msgID: 1230, nB=18, nRecv=3, tIntSec=5.08, bTout=0
```
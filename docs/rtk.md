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

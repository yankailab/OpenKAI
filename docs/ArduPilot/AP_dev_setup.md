# ArduPilot development

## Clone the repository

```bash
git clone --recurse-submodules https://github.com/ArduPilot/ardupilot.git
cd ardupilot
git submodule update --init --recursive
```

Run the following commands from the ArduPilot repository root.

## Install prerequisites

```bash
./Tools/environment_install/install-prereqs-ubuntu.sh -y
. ~/.profile
```

## Build SITL

```bash
./waf configure --board sitl
./waf copter
```

To clean build outputs:

```bash
./waf clean
```

## Run SITL

```bash
./Tools/autotest/sim_vehicle.py -v ArduCopter -f quad --map --console
```

### Use the Okushiga location

Open the locations file:

```bash
nano Tools/autotest/locations.txt
```

Add this entry:

```text
Okushiga=36.779180,138.527999,1500,0
```

Start one vehicle with the custom parameter file (adjust local paths as needed):

```bash
/home/kai/dev/ardupilot/Tools/autotest/sim_vehicle.py -v ArduCopter -f quad -L Okushiga --map --console --count=1 --add-param-file=/home/kai/ap_sitl_copter.parm
```

## Build and upload for CubeBlack

```bash
./waf configure --board CubeBlack
./waf copter
./waf --targets bin/arducopter --upload
```

## WebMavlinkStream with Tokyo Station SITL

After building Copter SITL in `/home/kai/dev/ardupilot`, run these commands from
the **OpenKAI repository root**, in separate terminals:

```bash
# Terminal 1: OpenKAI's two UDP listeners and browser viewer
cmake --build build -j2
build/OpenKAI jsonCfg/WebMavlinkStream.json
```

```bash
# Terminal 2: ArduCopter SITL and MAVProxy
docs/ArduPilot/run_web_mavlink_sitl.sh
```

Launch QGroundControl with automatic UDP connection enabled on **14550**.
Open **http://localhost:8080/** and click **Start** for visualization. Use QGC to
send commands; the browser viewer is read-only.

The [launcher](run_web_mavlink_sitl.sh) loads
[WebMavlinkStream.parm](../../jsonCfg/ardupilot/WebMavlinkStream.parm) with
`--add-param-file`. The file sets `SIM_OPOS_LAT=35.6812` and
`SIM_OPOS_LNG=139.7655`, approximately Tokyo Station's Marunouchi plaza,
`SIM_OPOS_ALT=5` m MSL and `SIM_OPOS_HDG=0` degrees (north). The altitude is a
chosen simulation ground level, not a surveyed elevation. It also sets vehicle
system ID **1**, MAVLink 2, and position/attitude/status stream rates.

Map windows and network destinations are `sim_vehicle.py` / MAVProxy launch
options, not ArduPilot parameters. The launcher omits `--map` and `--console`,
uses `--no-extra-ports` to disable implicit UDP outputs, and supplies:

| UDP destination | Listener | Purpose |
| --- | --- | --- |
| `127.0.0.1:14550` | QGC | Telemetry and commands |
| `127.0.0.1:14551` | OpenKAI `udpSitl` | Future bidirectional SITL handlers |
| `127.0.0.1:14552` | OpenKAI `udpMavlink` | Telemetry for the web viewer |

There are three destinations because OpenKAI has two separate UDP connections
and QGC has its own. The listeners are bound by OpenKAI and QGC; SITL's MAVProxy
forwards telemetry to them and relays replies. The web viewer then publishes
over HTTP/WebSocket on **8080**. `--mavproxy-args '--streamrate=-1'` leaves the
parameter-file stream rates in place; QGC may request different rates.

In [WebMavlinkStream.json](../../jsonCfg/WebMavlinkStream.json), `udpSitl` connects
`sitl_rx` / `sitl_tx` through `mavlinkSitl` to `sitlVehicle`. Its `bW2R:true`
learns MAVProxy's return address when telemetry arrives. Future handlers should
use this stream. The existing `vehicle` stream remains dedicated to visualization;
the two decoders use channels **1** and **0**, filtering vehicle system/component
**1/1**. Sending commands from OpenKAI still requires completing the existing
`MavMsgBase::getMsgQueue` TODO. Future setters should receive explicit sender IDs
**200/191**; the decoder's `mySystemID` / `myComponentID` fields do not currently
apply them to encoded messages.

The launcher reuses the existing build with `--no-rebuild` and stores EEPROM,
logs and other simulation files under `build/sitl-web-mavlink`. Saved
`eeprom.bin` parameters override new `.parm` defaults. For a clean parameter
reset of this dedicated simulation, run:

```bash
docs/ArduPilot/run_web_mavlink_sitl.sh -w
```

Use `ARDUPILOT_DIR=/path/to/ardupilot` to select another checkout, or
`SITL_RUN_DIR=/path/to/state` to select another simulation state directory.
Additional launcher arguments are passed to `sim_vehicle.py`; `-w` resets the
selected directory's saved parameters. Rebuild ArduPilot separately after
changing its code.

See the [viewer guide](../../html/viewer/mavlink/README.md) for model assets and
MSL/ellipsoid height alignment, and the official
[SITL usage](https://ardupilot.org/dev/docs/using-sitl-for-ardupilot-testing.html)
and [MAVProxy forwarding](https://ardupilot.org/mavproxy/docs/getting_started/forwarding.html)
documentation for launch and connection options.

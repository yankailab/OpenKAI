# `_FTservo`

`_FTservo` controls Feetech SMS/STS protocol servos through the external
[FTServo_Linux repository](https://github.com/ftservo/FTServo_Linux).
It replaces `_Feetech` and the bundled `src/Dependencies/Feetech` sources.
The driver uses the upstream `SMS_STS` API; SCSCL and HLSCL devices need their
own protocol-specific driver.

## Build

`USE_FTSERVO` defaults to `OFF`. Enabling it also enables actuator modules.
OpenKAI compiles the checkout's `src/*.cpp` and includes its `src/*.h` directly
in its own build. Do not build or install FTServo_Linux separately.

The default `FTSERVO_ROOT` is the sibling directory `../FTServo_Linux`.
For this workspace, the checkout is `/home/kai/dev/FTServo_Linux`.
To obtain it in a new workspace:

```bash
git clone https://github.com/ftservo/FTServo_Linux.git ../FTServo_Linux
```

From the OpenKAI repository:

```bash
cmake -S . -B build-ftservo \
  -DUSE_FTSERVO=ON \
  -DFTSERVO_ROOT=/home/kai/dev/FTServo_Linux \
  -DBUILD_EXECUTABLE=ON \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build-ftservo -j4
```

Change or omit `FTSERVO_ROOT` for another checkout location. An enabled build
reports a configuration error if the required source files are missing; a
disabled build does not need the checkout. Replace the old `USE_FEETECH` option
in existing build commands with `USE_FTSERVO`, change module `class` values to
`_FTservo`, and use the shared actuator `ID` field instead of the old `iID`.

## Test configuration

[`jsonCfg/FTservo.json`](https://github.com/yankailab/OpenKAI/blob/master/jsonCfg/FTservo.json)
follows the console/module structure of `jsonCfg/DDSM.json`. `_FTservo` owns its
serial port, so it does not need a `_SerialPort` module or a `BytePacketStream`.
Use one `_FTservo` instance per serial port.

Set `port`, `baud`, and `ID` to match the connected servo, then run:

```bash
./build-ftservo/OpenKAI jsonCfg/FTservo.json
```

The example starts with torque and movement disabled and displays feedback in
the console. To test position motion, set `bfSetPower` and `bfMove` to `true`
and choose a suitable `pTarget` before restarting. For wheel motion, use
`mode: 1` and a signed `sTarget`. The position example assumes a 4096-count
servo; adjust ranges to the actual model and mechanism.

| Field | Meaning |
| --- | --- |
| `ID` | Existing servo address, 0–253. Broadcast IDs are rejected. |
| `baud` | Host baud rate: 9600, 19200, 38400, 57600, 115200, 500000, or 1000000. Must already match the servo. |
| `mode` | `0`: position; `1`: continuous wheel speed. Applied on connection and after `setMode()`. |
| `bfSetPower` | Requested torque state, also controlled by `power(bool)`. |
| `bfMove` | Enables periodic motion commands, also controlled by `move(bool)`. |
| `pTarget`, `pRange`, `pErr` | Position target, limits, and completion tolerance in native servo counts. |
| `sTarget`, `sRange` | Native speed units; signed in wheel mode, magnitude in position mode. Position speed 0 has the SDK's unrestricted-speed meaning. |
| `aTarget`, `aRange` | Native acceleration units, bounded to 0–255 when transmitted. |
| `pOrigin`, `bfGotoOrigin` | Position requested by `gotoOrigin()`; enables movement once accepted with torque on. `move(false)` can stop it. |
| `tIntReadStatus` | Feedback interval in nanoseconds; default 100000000 (100 ms). |
| `tIntSendCMD` | Command interval in nanoseconds; default 20000000 (20 ms). |

Targets are limited by configured ranges and the protocol's signed 15-bit
position/speed magnitudes before conversion. The driver uses the current
`_ActuatorBase` position, speed, acceleration, current, and command/status flags.
Position, signed speed, signed current, and movement state come from one
validated feedback packet; units are model-specific, with no SI conversion.
Targets remain active until changed or movement/power is disabled.

`move(false)` holds the freshly read position in position mode or sends zero
speed in wheel mode. Torque enable first replaces any old hardware goal with a
hold/zero-speed command. Mode changes disable torque before changing modes.
Shutdown requests torque off and closes the port. Communication or servo errors
clear readiness, close the connection, and retry after one second; reconnect
reapplies the requested mode and power/movement state. `clearAlarm()` clears the
software alarm only after valid, fault-free feedback; it does not reset hardware.
ID/baud EEPROM programming is not implemented (`bfSetID: true` is rejected).

The OpenKAI serial adapter fixes descriptor cleanup and bounds reads after a
disconnect without modifying the external checkout.

## Regression tests

With the build above:

```bash
python3 test/run_ftservo_tests.py build-ftservo
```

These tests use a pseudo-terminal servo simulator and the actual external
protocol library. They require no servo hardware.

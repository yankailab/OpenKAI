# AProver LiDAR lawnmower rover

Build the `AProver_LiDAR` sample with ArduPilot/MAVLink, Universe, OpenCV and
Scepter support enabled. From the repository root:

```sh
cmake --build build --target AProver_LiDAR -j 4
build/samples/AProver_LiDAR/AProver_LiDAR samples/AProver_LiDAR/AProver_LiDAR.json
```

Set `udpAP.addrRemote`/`portRemote` to the rover's MAVLink endpoint before launch.
For a serial connection, enable `serialAP`, disable `udpAP`, and select the
serial streams in `apMavlink`.

## Configure, then operate

Replace `ROVER` below with the backend's hostname or IP address. Keep the same
backend process running through all three steps.

1. Open **http://ROVER:8082/** for the Scepter viewer. Click **Start**, leaving
   the camera module as `scepter` and command port as `7890`. Configure the
   sensor, keep depth capture and point-cloud output (`bDepth`, `bPCL`) enabled,
   then click **Save config** and close the page. Saved settings update the
   `scepter` section of the original launch JSON.
2. Open **http://ROVER:8080/** for the selectable grid viewer and click **Start**.
   Set the grid origin/size if needed. Pick the proximity volumes in front of
   the rover, then click **Send** in the cell picker and close the page. This
   saves `octGrid.vPorigin`, `octGrid.vRootCellSize` and
   `octGrid.vSelectedCells` in the same JSON and applies the selection immediately.
   **Load** retrieves the running selections. Both viewers consume `scPC`, the
   Scepter point-cloud stream used for collision detection.
3. Open **http://ROVER:8081/AProver/aprover.html** to operate the rover. **Load**
   retrieves the saved geofence; otherwise tap at least three map points around
   the boundary in order, without crossing edges, and select **Set** to save it. **Start** starts auto mode once a valid fence and
   fresh position/heading telemetry are available. Hold a direction for manual
   movement; release stops after the button timeout. Tap **STOP** to enter
   standby. Cutter **ON/OFF** controls the cutter independently.

The command connection allows one browser at a time; close each configuration
page before opening the next. Each viewer is served on its own stream port so
its existing host/port defaults work without changes to the shared viewer code.
When opening a viewer's `index.html` directly, enter port `8082` for Scepter or
`8080` for the grid before **Start**. Do not launch the viewers through the static
operation server on port `8081` unless you also change their stream port.

## Motion and settings

Auto drives forward while the selected cells and predicted position are clear.
An obstacle causes a pivot until it clears. A fence encounter chooses the pivot
direction from the incoming heading and the reflected heading at the boundary.
A bumper contact reverses until released, then pivots by `bumperTurnDeg` from
the heading at release before resuming normal avoidance. An emergency button
press enters standby and switches the cutter off; releasing it does not resume
motion or the cutter. Auto also stops with the cutter off if its fence or
position/heading telemetry becomes invalid.

| Section/key | Meaning |
| --- | --- |
| `apRover.speedGo`, `steerTurn` | Forward/reverse and pivot magnitudes, greater than 0 and at most 1. |
| `apRover.octGridOccu` | Obstacle when any selected cell contains more than this number of points. |
| `apRover.bumperTurnDeg` | Signed heading offset after bumper release: positive right, negative left, within −180–180°. |
| `apRover.hdgToleranceDeg` | Heading tolerance for completing the bumper turn. |
| `apRover.tOutBtn` | Manual command timeout in nanoseconds; default 200 ms. The UI repeats held commands every 100 ms. |
| `apRover.tOutNav` | Maximum auto position/heading age in nanoseconds; default 2 s. |
| `apRover.apModeMove` | ArduRover driving mode; default 0 (MANUAL). The controller requests this mode and arming when moving. |
| `apRover.iRCsteer`, `iRCthrottle`, `iRCservo` | One-based MAVLink RC override channels. |
| `apRover.pwmM`, `pwmD` | Neutral PWM and command range. |
| `apRover.pwmServoON`, `pwmServoOFF` | Cutter channel PWM values. |
| `geoFence.estD` | Forward prediction distance in metres. |

The hardware reports the emergency button in `BUTTON_CHANGE.state` bit 0 and
the bumper in bit 1 (set means pressed). Turning uses steering with neutral
throttle, matching the sample's skid-steer pivot behavior; the ArduRover motor
configuration must support it. `GLOBAL_POSITION_INT` supplies latitude,
longitude and heading; unknown headings cannot start or continue auto mode.

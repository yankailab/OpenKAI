#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "$script_dir/../.." && pwd)"
ardupilot_dir="${ARDUPILOT_DIR:-/home/kai/dev/ardupilot}"
sitl_run_dir="${SITL_RUN_DIR:-$repo_root/build/sitl-web-mavlink}"

# Keep EEPROM/logs separate from other simulations. No --map or --console.
# QGC listens on 14550; OpenKAI control on 14551; viewer telemetry on 14552.
# These are UDP destinations; MAVProxy accepts replies on each sending socket.
exec "$ardupilot_dir/Tools/autotest/sim_vehicle.py" \
  -v ArduCopter -f quad --no-rebuild --no-extra-ports \
  --use-dir "$sitl_run_dir" \
  --add-param-file "$repo_root/jsonCfg/ardupilot/WebMavlinkStream.parm" \
  --out udpout:127.0.0.1:14550 \
  --out udpout:127.0.0.1:14551 \
  --out udpout:127.0.0.1:14552 \
  --mavproxy-args '--streamrate=-1' \
  "$@"

# Timestamp units

OpenKAI stores host timestamps and elapsed durations in nanoseconds. `getTns()`
uses `CLOCK_MONOTONIC`; timestamps are not Unix time and exclude system suspend.
Use `nsec2sec<T>()` and `sec2nsec()` at interfaces that use seconds.

Point-cloud and IMU capture timestamps also use nanoseconds. Sensor clock origins
are preserved: Orbbec timestamps are multiplied by `NSEC_USEC` on input, while
Livox timestamps already use nanoseconds. Pair sensor streams only when they
share a capture clock. Geometry expiry against `getTns()` requires timestamps
on the host monotonic clock; set expiry to zero for unaligned device clocks.

Configuration migrated from microseconds must multiply the previous value by
1000. Shipped configurations have been updated, including these renamed keys:

| Nanosecond key | Default |
| --- | ---: |
| `tConfidenceTimeoutNs` | `0` for navigation; `1000000000` for SLAM |
| `tMapUpdateNs` | `200000000` |
| `tIMUpairToleranceNs` | `5000000` |
| `tCmdTimeoutNs` | `1000000000` |

The existing keys `dTexpire`, `dTexpireCell`, `dTexpirePCL`, `tOutBtn`,
`tOutTargetNotFound`, `tIntCheckAlarm`, `tIntReadStatus`, and `tIntSendCMD` now
contain nanoseconds. `_PCsend.tInt` and `_JSONbase.ieSendHB` also use nanoseconds.
ArduPilot heartbeat/message configuration intervals, `ieSendSec`, and `tOutSec`
continue to use seconds. SDK exposure settings, Modbus timeouts, and MAVLink
wire fields keep the units required by their external APIs.

GLIM timestamp status fields use the `Ns` suffix and decimal strings to preserve
64-bit values in JavaScript. `frameIntervalMs`, `processingMs`, and `workMs`
remain milliseconds for display. The GLIM submap protocol is version 2 (`GLM2`)
and the geometry protocol is version 6 (`W3D6`); update backend and browser assets
together.

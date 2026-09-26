Run these checks against a configured OpenKAI executable build with a compilation
database (`-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`):

```sh
python3 test/configLifecycle/run.py build
```

The runner supports CMake's Ninja and Unix Makefiles generators. It builds the
application, then compiles the test with the same feature switches and links its
entry point against the application objects. Use `--skip-build` after a separate
successful build. All configuration files and executables are created in a
temporary directory. No devices are opened or module worker threads started.

The checks cover manager creation/loading/linking, embedded thread and state
ownership, thread defaults, sequential saves to launch/include files, preservation
of unrelated settings, and write failure propagation. Enabled Universe, Scepter,
Orbbec, and GLIM features additionally exercise save/reload round trips and viewer
save commands. Optional cases compile only when their dependencies are enabled
in that build.

Export-control checks verify that `saveConfig(false)` updates the bound JSON
without changing files, including when the destination cannot be written.
They cover BASE, inherited geometry settings, and embedded threads;
`saveConfig(true)` then exports the complete updated configuration.

Additional save-coverage checks change runtime scalar/vector settings, camera and
RGBD settings, named pipeline filters, nested ports and threads, full-width radio
addresses, and fractional RTCM intervals. They verify saved files and reloads,
including settings removed from the source JSON before saving.

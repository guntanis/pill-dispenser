# Contributing

Thanks for looking at this. The project is at the stage where real-world build reports are worth more than code.

## Build reports

If you print a module or wire up the electronics, open an issue with:

- The pill you tested with (length x width x thickness in mm, tablet or capsule) and the `params.scad` values you used.
- How many cycles you ran and how many singles, doubles and misses you saw.
- Photos of jams or of pills wedging anywhere.
- Printer, material and layer height.

## Pull requests

- Firmware changes must build with `pio run` from the `firmware/` directory. Say in the PR whether you ran it on hardware.
- CAD changes must open in OpenSCAD without errors or warnings in the console. Keep every user-tunable dimension in `cad/params.scad`; the part files should only read from it.
- Wiring or pin changes must be made in both `docs/wiring.md` and `firmware/src/config.h` in the same PR.
- Never commit `firmware/include/secrets.h` or any WiFi credentials, ntfy topics or device IPs.
- Keep the README's status table honest. If something is still unverified, say so.

## Scope

In scope: anything that makes single-pill dispensing more reliable, the firmware safer, or the build easier to reproduce.

Out of scope for now: cloud services, mobile apps, anything that would make the device look more finished than it is.

# Pill-metering module (OpenSCAD)

One module of a 5-slot pill dispenser: hopper -> rotating disk with one pocket -> wiper -> drop hole -> chute.
28BYJ-48 stepper hangs under the base plate.

NOTE: this design has never been rendered. On first open, check OpenSCAD's
console for errors/warnings and fix any before printing.

## Setup
- Edit pill dimensions (`pill_type`, `pill_length`, `pill_width`, `pill_thickness`) in `params.scad`.
  A capsule preset is in a comment there.
- Preview/export via the OpenSCAD GUI: open `assembly.scad`, set `part` (e.g. "disk"), render (F6), export STL.
- Or run `./export.sh` (writes `stl/*.stl`). Set `OPENSCAD=/path/to/openscad` if not on PATH;
  macOS fallback: `/Applications/OpenSCAD.app/Contents/MacOS/OpenSCAD`.

Parts: disk, base_plate, hopper, hopper_lid, chute (printable); stepper (reference only).

## Print settings
- PETG for pill-contact parts (disk, hopper, plate), 0.2 mm layers.
- Disk: pocket side up. Hopper: upright (flange down). Plate: recess side up.

## Operation
- Disk rotates CLOCKWISE viewed from above. Hopper outlet at +X (0 deg), drop hole at -X (180 deg).
- The wiper slot is on the trailing -Y edge of the outlet; insert a flexible strip (e.g. thin PET/silicone).

## Tuning
- Double pills: make the pocket shallower, or use a stiffer wiper.
- Missed pills: add +0.2 mm pocket clearance, and/or agitate the disk in firmware (small back-and-forth).

- `disk_gap` (disk top sits this far below the plate top; the wiper reaches through it): if pills wedge in
  the gap, reduce it; if the flange rubs the disk, increase it.

## Known caveats
- Chute top end is cut at 45 degrees, not flat to the plate.

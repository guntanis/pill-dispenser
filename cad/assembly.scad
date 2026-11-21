// Assembly / part selector. Set part="disk" etc. (or via -D 'part="disk"').
include <params.scad>
use <lib/stepper_28byj48.scad>
use <disk.scad>
use <base_plate.scad>
use <hopper.scad>
use <chute.scad>

part = "all";

hopper_top = flange_t + hopper_taper_h + hopper_h;

if (part == "disk") {
    disk();
} else if (part == "base_plate") {
    base_plate();
} else if (part == "hopper") {
    hopper();
} else if (part == "hopper_lid") {
    hopper_lid();
} else if (part == "chute") {
    chute();
} else if (part == "stepper") {
    stepper_28byj48();
} else {
    // full assembly; plate bottom at z=0, plate top at z=plate_h
    color("gray") base_plate();
    // stepper hangs under the plate; shaft tip rises into the disk socket
    color("silver") translate([0, 0, 0]) stepper_28byj48();
    // disk sits in the recess, top flush with plate top
    color("steelblue") translate([0, 0, plate_t]) disk();  // top sits disk_gap below plate top
    // hopper flange on plate top
    color("orange", 0.6) translate([0, 0, plate_h]) hopper();
    // lid on top of hopper tube (1 mm clearance built into lid)
    color("tomato", 0.6)
        translate([pocket_r - hopper_offset, 0, plate_h + hopper_top - 7]) hopper_lid();
    // chute under the drop hole
    color("seagreen", 0.7) translate([0, 0, 0]) chute();
}

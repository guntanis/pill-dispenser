// Fixed base plate. Origin at disk center, bottom at z=0, top at z=plate_h.
include <params.scad>
use <lib/stepper_28byj48.scad>

module base_plate() {
    difference() {
        translate([-plate_w / 2, -plate_w / 2, 0]) cube([plate_w, plate_w, plate_h]);
        // disk recess in top (disk top flush with plate top)
        translate([0, 0, plate_t]) cylinder(d = recess_d, h = recess_depth + 1);
        // drop hole at 180 deg, through everything
        translate([pocket_r * cos(180), pocket_r * sin(180), -1])
            cylinder(d = drop_hole_d, h = plate_h + 2);
        // stepper ear holes + shaft hole
        stepper_mount_holes(plate_h);
        // hopper bolt holes (insert bores) in the plate top, outside the recess
        for (a = [90, 210, 330])
            translate([hopper_bolt_circle_d / 2 * cos(a), hopper_bolt_circle_d / 2 * sin(a), plate_h - 8])
                cylinder(d = insert_bore_d, h = 9);
        // enclosure corner holes
        for (sx = [-1, 1], sy = [-1, 1])
            translate([sx * (plate_w / 2 - corner_inset), sy * (plate_w / 2 - corner_inset), -1])
                cylinder(d = corner_hole_d, h = plate_h + 2);
    }
}

base_plate();

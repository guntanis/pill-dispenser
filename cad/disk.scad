// Metering disk. Origin at disk center, bottom at z=0.
include <params.scad>
use <lib/stepper_28byj48.scad>

module pocket_shape(extra = 0) {
    // 2D footprint of the pocket (grown by `extra` per side)
    if (pill_type == "tablet") {
        circle(d = pocket_w + 2 * extra);
    } else {
        // slot oriented tangentially (along Y)
        hull() {
            translate([0,  (pocket_l - pocket_w) / 2]) circle(d = pocket_w + 2 * extra);
            translate([0, -(pocket_l - pocket_w) / 2]) circle(d = pocket_w + 2 * extra);
        }
    }
}

module disk() {
    difference() {
        cylinder(d = disk_d, h = disk_t);
        // pocket
        translate([pocket_r, 0, disk_t - pocket_depth])
            linear_extrude(pocket_depth + 1) pocket_shape();
        // 0.5 mm chamfer on lip
        translate([pocket_r, 0, disk_t - 0.5])
            hull() {
                linear_extrude(0.01) pocket_shape();
                translate([0, 0, 0.5]) linear_extrude(0.01) pocket_shape(0.5);
            }
        // D-shaft socket from below, blind
        translate([0, 0, -0.01])
            d_shaft(shaft_d + 2 * shaft_clear, shaft_flat + 2 * shaft_clear, disk_t - 1);
    }
}

disk();

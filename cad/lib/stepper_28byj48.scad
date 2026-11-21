// 28BYJ-48 reference body. Shaft axis at the origin; body offset +shaft_offset in X, ears along Y.
include <../params.scad>

module d_shaft(d = shaft_d, flat = shaft_flat, h = shaft_len) {
    // D profile: circle d, cut to `flat` across-flats thickness (flats on +/-Y)
    intersection() {
        cylinder(d = d, h = h);
        translate([-d, -flat / 2, -1]) cube([2 * d, flat, h + 2]);
    }
}

module stepper_28byj48() {
    // body hangs below z=0 (top of body at z=0), shaft up from z=0
    translate([shaft_offset, 0, -motor_body_h]) cylinder(d = motor_body_d, h = motor_body_h);
    // mount ears along Y
    translate([shaft_offset, 0, -1.0])
        hull() {
            translate([0, -motor_ear_span / 2, 0]) cylinder(d = 7, h = 1);
            translate([0,  motor_ear_span / 2, 0]) cylinder(d = 7, h = 1);
        }
    // shaft
    d_shaft();
}

module stepper_mount_holes(h) {
    // two ear holes and shaft hole, cutters spanning z = -1 .. h
    for (s = [-1, 1])
        translate([shaft_offset, s * motor_ear_span / 2, -1])
            cylinder(d = motor_ear_hole, h = h + 2);
    translate([0, 0, -1]) cylinder(d = shaft_d + 1, h = h + 2);
}

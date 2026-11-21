// Hopper and lid. Origin at disk center; flange bottom at z=0 (sits on plate top).
include <params.scad>

hx = pocket_r - hopper_offset;   // tube center X
tube_z0 = flange_t + hopper_taper_h;

module hopper_outer() {
    union() {
        translate([0, 0, 0]) cylinder(d = hopper_flange_d, h = flange_t);
        hull() {
            translate([pocket_r, 0, 0]) cylinder(d = hopper_outlet_d + 2 * hopper_wall, h = 0.01);
            translate([hx, 0, tube_z0 - 0.01]) cylinder(d = hopper_id + 2 * hopper_wall, h = 0.01);
        }
        translate([hx, 0, tube_z0 - 0.01]) cylinder(d = hopper_id + 2 * hopper_wall, h = hopper_h);
    }
}

module hopper_inner() {
    hull() {
        translate([pocket_r, 0, -1]) cylinder(d = hopper_outlet_d, h = 0.01);
        translate([hx, 0, tube_z0 - 0.01]) cylinder(d = hopper_id, h = 0.01);
    }
    translate([hx, 0, tube_z0 - 0.02]) cylinder(d = hopper_id, h = hopper_h + 1);
    translate([pocket_r, 0, -1]) cylinder(d = hopper_outlet_d, h = flange_t + 2);
}

module hopper() {
    difference() {
        hopper_outer();
        hopper_inner();
        // flange bolt holes
        for (a = [90, 210, 330])
            translate([hopper_bolt_circle_d / 2 * cos(a), hopper_bolt_circle_d / 2 * sin(a), -1])
                cylinder(d = bolt_d, h = flange_t + 2);
        // wiper slot on trailing (-Y) edge of outlet, running along X, open at bottom
        translate([pocket_r - wiper_len / 2, -hopper_outlet_d / 2 - wiper_w, -0.01])
            cube([wiper_len, wiper_w, wiper_depth]);
    }
}

module hopper_lid() {
    lid_d = hopper_id + 2 * hopper_wall + 2 * 1 + 2 * 2; // 1 mm clearance + 2 mm skirt wall
    lid_top = 2;
    skirt_h = 8;
    difference() {
        union() {
            cylinder(d = lid_d, h = lid_top + skirt_h);
        }
        // recess over hopper rim
        translate([0, 0, -0.01]) cylinder(d = hopper_id + 2 * hopper_wall + 2, h = skirt_h);
    }
    // desiccant cup hanging from the underside (lid is centered on its own origin)
    desiccant_cup();
}

module desiccant_cup() {
    // 20x20x8 pocket cut into the lid underside (lid made thick enough locally)
    // Implemented as a separate hanging cup with a perforated grid floor.
    translate([0, 0, 0]) difference() {
        translate([-11, -11, 2]) cube([22, 22, 8]);
        translate([-10, -10, 1.99]) cube([20, 20, 8.01]);
    }
    // 1 mm grid retainer across the open face
    for (i = [-9 : 3 : 9]) {
        translate([i - 0.5, -10, 2]) cube([1, 20, 1]);
        translate([-10, i - 0.5, 2]) cube([20, 1, 1]);
    }
}

hopper();

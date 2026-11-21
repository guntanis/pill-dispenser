// Chute: 10 mm vertical section under the drop hole, then a 45 deg run toward -X.
// Origin: plate bottom at z=0; drop hole center at (-pocket_r, 0). Duct hangs below z=0.
include <params.scad>

outer = chute_inner + 2 * chute_wall;   // 18 -> y span +/-9
straight_len = 10;

module duct(l) {
    // hollow square duct along +Z from z=0 to z=l
    difference() {
        translate([-outer / 2, -outer / 2, 0]) cube([outer, outer, l]);
        translate([-chute_inner / 2, -chute_inner / 2, -1]) cube([chute_inner, chute_inner, l + 2]);
    }
}

module chute() {
    translate([-pocket_r, 0, 0]) {
        // vertical section, top face flush with plate underside (z=0)
        translate([0, 0, -straight_len]) duct(straight_len);
        // angled run from the bottom of the vertical section, heading down and toward -X
        translate([0, 0, -straight_len])
            rotate([0, 180 + chute_angle, 0]) duct(chute_len);
    }
}

chute();

// Shared parameters for the pill-metering module. All units mm.
$fn = 72;

pill_type = "tablet";      // "tablet" or "capsule"
pill_length = 8;           // pill length
pill_width = 8;            // pill width
pill_thickness = 4;        // pill thickness
// Capsule preset: pill_type="capsule"; pill_length=21.7; pill_width=7.6; pill_thickness=7.6;

pocket_w = pill_width + 0.5;          // pocket width
pocket_l = pill_length + 0.5;         // pocket length
pocket_depth = pill_thickness + 0.3;  // pocket depth

disk_d = 64;                          // disk diameter
disk_gap = 0.5;                       // disk top sits this far below plate top (wiper reaches through)
disk_floor = 2;                       // material under pocket
disk_t = pocket_depth + disk_floor;   // disk thickness
pocket_r = 25;                        // disk center to pocket center
disk_recess_clear = 0.3;              // radial clearance in plate recess

plate_w = 90;                         // plate square size
plate_t = 5;                          // plate thickness under disk
recess_depth = disk_t + disk_gap;     // recess depth in plate top
plate_h = plate_t + recess_depth;     // total plate block height
recess_d = disk_d + 2 * disk_recess_clear; // recess diameter

drop_hole_d = max(pocket_l, pocket_w) + 1; // drop hole diameter (at 180 deg)

hopper_id = 36;                     // hopper inner diameter
hopper_wall = 2;                      // hopper wall thickness
hopper_h = 60;                        // hopper tube height
hopper_outlet_d = max(pocket_l, pocket_w) + 2; // outlet diameter at (pocket_r,0)
hopper_taper_h = 20;                  // tapered bottom height
hopper_offset = 7;                    // tube center = pocket_r - 7 = 18
hopper_flange_d = 84;                 // flange diameter
flange_t = 3;                         // flange thickness
hopper_bolt_circle_d = 76;            // bolt circle diameter (r=38)
bolt_d = 3.4;                         // hopper flange bolt hole
insert_bore_d = 4.5;                  // plate bore (heat-set insert)
corner_hole_d = 3.4;                  // enclosure mount holes
corner_inset = 5;                    // corner hole inset from edge

wiper_w = 1.2;                        // wiper slot width
wiper_depth = 10;                     // wiper slot depth
wiper_len = hopper_outlet_d + 4;      // wiper slot length (along X)

motor_body_d = 28;                    // 28BYJ-48 body diameter
motor_body_h = 19;                    // body height
motor_ear_span = 35;                  // mount ear spacing
motor_ear_hole = 4.2;                 // ear hole diameter
shaft_d = 5;                          // D-shaft diameter
shaft_flat = 3;                       // across-flats dimension
shaft_offset = 8;                     // shaft offset from body center
shaft_len = 10;                       // shaft length
shaft_clear = 0.2;                    // socket clearance

chute_inner = 14;                     // chute inner square
chute_wall = 2;                       // chute wall
chute_angle = 45;                     // outward angle
chute_len = 50;                       // chute length

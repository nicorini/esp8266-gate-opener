// Enclosure for the gate opener: NodeMCU v3 LoLin ("new NodeMCU V3", CH340G) +
// Ebyte E07-M1101D-SMA v2.0 (CC1101) + 0.96" 128x64 I2C OLED (SSD1306, GM009605)
// in the lid + 12x12 mm tactile switch with a round cap, wired with female dupont
// jumpers. It stands on a shelf or table with the button and the straight antenna
// on top.
// Part of github.com/nicorini/esp8266-gate-opener. All dimensions in mm.
//
// Print: PETG, 0.2 mm layers, 3 perimeters, 20-30 % infill, no supports.
// The box prints open side up; part = "lid" is already flipped for printing.
// STL files and the README images: sh hardware/render.sh
//
// Before printing
//   1. Measure your parts with a caliper and fix every value marked MEASURE.
//      Defaults come from the Ebyte datasheet and published NodeMCU drawings;
//      NodeMCU clones vary by up to 1 mm.
//   2. Power the bare electronics where the box will sit, antenna vertical, and
//      check that the gate opens from there. Keep it 10-20 cm from metal
//      (fridge, radiator, metal frames).
//   3. Print part = "test" (a 6 mm ring plus the lid) and tune lid_tol until the
//      lid clicks in and stays.
//
// Assembly
//   - NodeMCU: plug the jumpers first, then press the board onto the floor pegs,
//     USB towards the right wall, so the D1-D8 row faces the CC1101. Two fingers
//     under the lid keep it there.
//   - CC1101: plug its jumpers, lower it SMA up next to the right wall, push it
//     onto the two wall pegs, components facing the wall.
//   - Button: the tactile switch sits on the pillar in the back left corner, legs
//     hanging down on its left and right, a drop of hot glue under the body. The
//     pillar takes the press, the lid only guides the cap.
//   - OLED: glass towards the lid window, header towards the back wall, pressed
//     into the frame under the lid, a drop of hot glue on the PCB corners. The
//     flex cable folds round the front edge, where the frame leaves room. Wire
//     GND to G, VDD to 3V, SCK to D4, SDA to D3 (D1 and D2, the default I2C
//     pins, are taken by the CC1101 and the button).
//   - Lid: cap and SMA through their holes. Screwing the antenna on clamps the lid
//     between the antenna and the SMA body: unscrew the antenna to open the box.
//   - Wiring: see README.md. 10 cm jumpers fit best; bundle longer ones.
//   - Optional: stick-on rubber feet under the box.

// Open Window > Customizer in OpenSCAD to change these with menus, boxes and sliders.
/* [View] */
part = "both";        // [both:Preview, box:Box (export), lid:Lid (export), test:Fit test (export)]
// Preview only: how far the lid floats above the box (0 = closed)
lid_lift = 12;        // [0:1:80]
// Preview only: cut away everything in front of this depth to look inside (0 = off)
cut_y = 0;            // [0:1:60]
// Preview only: lid opacity (1 = solid)
lid_opacity = 0.6;    // [0.1:0.1:1]
show_box = true;
show_lid = true;
// Simplified electronics, preview only, never exported
show_nodemcu = true;
show_cc1101 = true;
show_oled = true;
show_button = true;
show_antenna = true;

/* [Shell] */
wall = 2.0;           // 4 lines with a 0.4 mm nozzle; floor too
r_out = 3;            // outer corner radius
lid_t = 2.0;          // lid plate
lip_h = 3;            // lid lip that goes into the box
lip_t = 1.6;
lid_tol = 0.15;       // lip-to-wall gap per side; too tight -> raise, too loose -> lower
lid_bump = 0.35;      // retention bumps on the lip
$fn = 48;

/* [NodeMCU v3 LoLin] */
mcu_l = 58;           // MEASURE: 57.8-59 depending on the clone
mcu_w = 31.5;         // MEASURE: 31-32
pcb_t = 1.6;          // MEASURE
pcb_tol = 0.3;        // gap around the PCB in the corner pockets
hole_dx = 52.3;       // MEASURE: mounting holes, centre to centre (photo estimate)
hole_dy = 25.5;       // MEASURE
mcu_peg_d = 2.0;      // MEASURE the holes: ~2.5 on this board, peg 0.5 smaller
peg_h = 2.5;
usb_over = 1.1;       // MEASURE: micro-USB overhang past the PCB edge
usb_w = 12.5;         // cutout for the micro-USB plug's overmould
usb_h = 9;            // 9, not 8: the bridge above it sags a little
usb_dz = 1.5;         // connector centre above the PCB top

/* [CC1101 E07-M1101D-SMA, Ebyte datasheet v1.20] */
cc_l = 30;  cc_w = 15;  cc_pcb_t = 1.2;
cc_hole_x = 7.5;      // MEASURE: mounting holes (dia 3) from the SMA edge; the
                      //   datasheet says 10, the photo of this board ~7.5
cc_hole_y = 2.7;      //   and from each long edge
sma_l = 9.6;          // MEASURE: SMA length past the PCB edge
sma_body_l = 1.5;     // MEASURE: thickness of the flat SMA flange at the PCB edge
sma_body_w = 7.0;     // MEASURE: width of that flange
sma_thread_d = 6.35;  // 1/4"-36
sma_d = 7;            // lid hole
cc_sma_dz = 1.8;      // MEASURE: SMA axis from the PCB face with the header pins
                      //   (the centre pin is soldered on the component face)
cc_peg_d = 2.4;       // for 3 mm holes

/* [Button: 12x12 tactile switch + round cap] */
btn_w = 12;           // MEASURE: switch body, square
btn_body_h = 3.8;     // MEASURE: switch body height, stem excluded
btn_top = 10.5;       // MEASURE: body underside (legs excluded) to the cap top, not pressed
btn_cap_d = 14.5;     // MEASURE: widest cap diameter (the ring at its base)
btn_proud = 2.5;      // cap above the lid
btn_post = 9;         // pillar top: the legs, 12.5 apart, hang outside it

/* [OLED 0.96" 128x64 I2C, GM009605] */
oled_pcb_w = 27.3;    // MEASURE: PCB along the header edge
oled_pcb_h = 27.8;    // MEASURE: PCB across it
oled_t = 3.7;         // MEASURE: glass face to PCB back, header spacer excluded
oled_fpc = 1.2;       // MEASURE: flex cable loop past the PCB edge opposite the header
oled_hdr_dy = 2.5;    // MEASURE: header pin row from the PCB edge
oled_view_w = 21.74;  // active area (SSD1306 0.96" panels: 21.74 x 10.86)
oled_view_h = 10.86;
oled_view_dy = 2.5;   // MEASURE: active area centre from the PCB centre, towards the header
mcu_top_h = 4.2;      // MEASURE: tallest part on the NodeMCU (ESP-12 module, raised)

// ---------- derived ----------
// Boards sit high enough for straight female dupont jumpers on the header pins.
hdr_spacer = 2.5;     // plastic spacer of the male headers
dupont_l = 14;        // female dupont housing
wire_bend = 8;        // room for the wire to turn past the housing
lift = hdr_spacer + dupont_l + wire_bend;

// NodeMCU: ESP-12 end at x = mcu_x0, USB end towards the right wall.
mcu_x0 = 1.5;
mcu_y0 = 1.5;
mcu_x1 = mcu_x0 + mcu_l;
cc_gap = 6.5;         // between the NodeMCU and the CC1101
cc_edge = lid_tol + lip_t + 0.3;  // CC1101 to back wall: its top corner clears the lid lip
in_x = mcu_x1 + usb_over + 0.7;
in_y = mcu_y0 + mcu_w + cc_gap + cc_w + cc_edge;
floor_t = wall;
in_h = max(lift + pcb_t + usb_dz + usb_h/2 + lip_h + 0.5,
           cc_l + sma_body_l + 1);
ox = in_x + 2*wall;  oy = in_y + 2*wall;  oz = floor_t + in_h;

// CC1101 stands vertical: PCB in a y-z plane, SMA body against the lid underside,
// header pins pointing -x. Inside coordinates.
// Wall standoff so the SMA flange clears the lid lip.
cc_standoff = cc_sma_dz - cc_pcb_t + sma_body_w/2 + 0.3 + lip_t + lid_tol;
cc_px = in_x - cc_standoff - cc_pcb_t;    // PCB face with the header pins
cc_y0 = in_y - cc_edge - cc_w;
cc_cy = cc_y0 + cc_w/2;
cc_z1 = in_h - sma_body_l;                // PCB top edge (SMA side)
cc_z0 = cc_z1 - cc_l;                     // PCB bottom edge (header side)
cc_hz = cc_z1 - cc_hole_x;                // mounting holes
sma_x = cc_px + cc_sma_dz;                // SMA axis
sma_out = sma_l - sma_body_l - lid_t;     // thread above the lid for the antenna

// Button in the free strip behind the NodeMCU, as far left as the lid lip allows.
lip_in = lid_tol + lip_t;                 // inner face of the lid lip, from the wall
btn_hole = btn_cap_d + 0.8;
btn_x = lip_in + 1 + btn_hole/2;
btn_y = (mcu_y0 + mcu_w + in_y)/2;
btn_seat = in_h + lid_t + btn_proud - btn_top;   // switch underside = pillar top
// OLED next to the button, header row over the free strip so its jumpers clear the
// NodeMCU; the rest of the module overhangs the NodeMCU, which sits low enough.
oled_frame = 1.2;
oled_x = btn_x + btn_hole/2 + 1.5 + oled_frame + 0.3 + oled_pcb_w/2;
oled_y = in_y - lip_in - oled_frame - 0.3 - oled_pcb_h/2;   // frame against the lip
oled_hdr_y = oled_y + oled_pcb_h/2 - oled_hdr_dy;
hx0 = mcu_x0 + (mcu_l - hole_dx)/2;
hy0 = mcu_y0 + (mcu_w - hole_dy)/2;

echo(str("Outside: ", ox, " x ", oy, " x ", oz + lid_t, " mm"));
echo(str("SMA thread above the lid: ", sma_out, " mm (want >= 3)"));
echo(str("Button pillar: ", btn_seat, " mm"));
assert(sma_out >= 2.5, "SMA thread too short for the antenna: check sma_l and sma_body_l");
assert(cc_z0 >= 0.5, "CC1101 hits the floor: raise in_h");
assert(in_h - oled_t >= lift + pcb_t + mcu_top_h + 0.5, "OLED touches the NodeMCU");
assert(oled_hdr_y - 1.27 >= mcu_y0 + mcu_w + 0.5, "OLED jumpers hit the NodeMCU");
assert(oled_x + oled_pcb_w/2 + 0.3 + oled_frame <= sma_x - sma_body_w/2 - 0.5,
       "OLED frame hits the SMA flange");
assert(btn_seat + btn_body_h <= in_h - 1, "Button body hits the lid: check btn_top");

// ---------- helpers ----------
module rbox(x, y, z, r) {
  hull() for (i = [r, x-r], j = [r, y-r]) translate([i, j, 0]) cylinder(r = r, h = z);
}
module inside() { translate([wall, wall, floor_t]) children(); }

module rrect_x(w, h, len, r = 1.5) {
  hull() for (j = [-w/2+r, w/2-r], k = [-h/2+r, h/2-r])
    translate([0, j, k]) rotate([0, 90, 0]) cylinder(r = r, h = len);
}

module peg(d) cylinder(d1 = d, d2 = d - 0.5, h = peg_h);

// Corner pocket for the NodeMCU: an L-shaped rim around a 5x5 seat.
// Posts closer than 2 mm to a wall are merged into it.
post_s = 5;
post_f = 1.2 + pcb_tol;
mcu_corners = [[mcu_x0, mcu_y0, -1, -1], [mcu_x1, mcu_y0, 1, -1],
               [mcu_x0, mcu_y0 + mcu_w, -1, 1], [mcu_x1, mcu_y0 + mcu_w, 1, 1]];

module corner_post(x, y, dx, dy) {
  q = [dx > 0 ? x - post_s : x - post_f,  dy > 0 ? y - post_s : y - post_f,
       dx > 0 ? x + post_f : x + post_s,  dy > 0 ? y + post_f : y + post_s];
  x0 = q[0] < 2 ? -0.5 : q[0];        x1 = in_x - q[2] < 2 ? in_x + 0.5 : q[2];
  y0 = q[1] < 2 ? -0.5 : q[1];        y1 = in_y - q[3] < 2 ? in_y + 0.5 : q[3];
  difference() {
    translate([x0, y0, 0]) cube([x1 - x0, y1 - y0, lift + 2.5]);
    translate([dx > 0 ? x - post_s - 1 : x - pcb_tol, dy > 0 ? y - post_s - 1 : y - pcb_tol, lift])
      cube([post_s + 1 + pcb_tol, post_s + 1 + pcb_tol, 10]);
  }
}

// Wall boss for one CC1101 hole: 4.5 mm square face, 45-degree underside so it
// prints without supports, horizontal peg into the hole.
module cc_boss(y) {
  d = cc_standoff;  s = 4.5;
  hull() {
    translate([in_x - d, y - s/2, cc_hz - s/2]) cube([d + 0.5, s, s]);
    translate([in_x - 0.01, y - s/2, cc_hz - s/2 - d]) cube([0.51, s, 0.01]);
  }
  translate([in_x - d, y, cc_hz]) rotate([0, -90, 0]) peg(cc_peg_d);
}

// ---------- box ----------
module box() {
  difference() {
    rbox(ox, oy, oz, r_out);
    translate([wall, wall, floor_t]) cube([in_x, in_y, in_h + 1]);
    // micro-USB
    inside() translate([in_x - 1, mcu_y0 + mcu_w/2, lift + pcb_t + usb_dz])
      rrect_x(usb_w, usb_h, wall + 2);
    // pry slot for the lid
    translate([ox/2 - 6, oy - wall - 1, oz - 1.5]) cube([12, wall + 2, 2]);
  }
  inside() {
    for (c = mcu_corners) corner_post(c[0], c[1], c[2], c[3]);
    for (i = [0, hole_dx], j = [0, hole_dy]) translate([hx0 + i, hy0 + j, lift]) peg(mcu_peg_d);
    for (y = [cc_y0 + cc_hole_y, cc_y0 + cc_w - cc_hole_y]) cc_boss(y);
    // button pillar, flared at the floor
    translate([btn_x, btn_y, 0]) hull() {
      translate([-btn_post/2, -btn_post/2, 0]) cube([btn_post, btn_post, btn_seat]);
      translate([-(btn_post + 6)/2, -(btn_post + 6)/2, 0]) cube([btn_post + 6, btn_post + 6, 0.01]);
    }
  }
}

// ---------- lid ----------
module lid() {
  finger_z = floor_t + lift + pcb_t + 0.3;          // 0.3 mm above the NodeMCU
  lip_x = in_x - 2*lid_tol;  lip_y = in_y - 2*lid_tol;
  difference() {
    union() {
      translate([0, 0, oz]) rbox(ox, oy, lid_t, r_out);
      translate([wall + lid_tol, wall + lid_tol, oz - lip_h]) {
        difference() {
          cube([lip_x, lip_y, lip_h]);
          translate([lip_t, lip_t, -1]) cube([lip_x - 2*lip_t, lip_y - 2*lip_t, lip_h + 2]);
        }
        // retention bumps on the long sides
        for (x = [lip_x/3, 2*lip_x/3], y = [0, lip_y])
          translate([x, y, lip_h/2]) scale([2, lid_bump, 1]) sphere(r = 1);
      }
      // OLED frame under the window, longer on the front for the flex loop
      translate([wall + oled_x, wall + oled_y - oled_fpc/2, oz - oled_t/2]) difference() {
        cube([oled_pcb_w + 0.6 + 2*oled_frame, oled_pcb_h + oled_fpc + 0.6 + 2*oled_frame, oled_t], center = true);
        cube([oled_pcb_w + 0.6, oled_pcb_h + oled_fpc + 0.6, oled_t + 1], center = true);
      }
      // fingers on the bare PCB near the ESP-12 end corners: 6.5 mm from the
      // end, 1.6 mm from the long edge (clear of pins, holes and resistors)
      for (y = [mcu_y0 + 1.6, mcu_y0 + mcu_w - 1.6])
        translate([wall + mcu_x0 + 6.5 - 1.25, wall + y - 1.25, finger_z])
          cube([2.5, 2.5, oz - finger_z + 0.01]);
    }
    // button cap
    translate([wall + btn_x, wall + btn_y, oz - 1]) {
      cylinder(d = btn_hole, h = lid_t + 2);
      // chamfer on the outer face, which prints on the bed
      translate([0, 0, 1 + lid_t - 0.4]) cylinder(d1 = btn_hole, d2 = btn_hole + 0.8, h = 0.41);
    }
    // SMA
    translate([wall + sma_x, wall + cc_cy, oz - 1]) cylinder(d = sma_d, h = lid_t + 2);
    // OLED window, 45-degree chamfer outwards
    translate([wall + oled_x, wall + oled_y + oled_view_dy, oz])
      hull() {
        translate([0, 0, -0.5]) cube([oled_view_w + 0.4, oled_view_h + 0.4, 1], center = true);
        translate([0, 0, lid_t]) cube([oled_view_w + 0.4 + 2*lid_t, oled_view_h + 0.4 + 2*lid_t, 0.02], center = true);
      }
  }
}

module lid_print() { translate([0, oy, oz + lid_t]) rotate([180, 0, 0]) lid(); }

// ---------- simplified electronics, preview only ----------
module parts(lid_dz = 0) {
  row_y = 1.25;                         // pin rows 1.25 mm from the long edges
  pin_x0 = 11.4;                        // first pin from the ESP-12 end; 15 per row
  hdr_w = 4*2.54;                       // CC1101 2x4 header, across the PCB
  inside() {
    // NodeMCU with jumpers on both rows
    if (show_nodemcu) translate([mcu_x0, mcu_y0, lift]) {
      color("darkgreen") cube([mcu_l, mcu_w, pcb_t]);
      color("silver") translate([0.8, (mcu_w - 16)/2, pcb_t]) cube([24, 16, 3]);
      color("silver") translate([mcu_l - 5.5 + usb_over, mcu_w/2 - 4, pcb_t]) cube([5.5, 8, 2.8]);
      for (y = [row_y, mcu_w - row_y]) {
        color("black") translate([pin_x0 - 1.27, y - 1.27, -hdr_spacer]) cube([15*2.54, 2.54, hdr_spacer]);
        color("dimgray") translate([pin_x0 - 1.27, y - 1.27, -hdr_spacer - dupont_l])
          cube([15*2.54, 2.54, dupont_l]);
      }
    }
    // CC1101 standing, header at the bottom, jumpers towards -x
    if (show_cc1101) translate([cc_px, cc_y0, cc_z0]) {
      color("royalblue") cube([cc_pcb_t, cc_w, cc_l]);
      color("black") translate([-hdr_spacer, (cc_w - hdr_w)/2, 0.3]) cube([hdr_spacer, hdr_w, 2*2.54]);
      color("dimgray") translate([-hdr_spacer - dupont_l, (cc_w - hdr_w)/2, 0.3])
        cube([dupont_l, hdr_w, 2*2.54]);
    }
    if (show_cc1101) translate([sma_x, cc_cy, cc_z1]) {
      color("gold") translate([0, 0, sma_body_l/2]) cube([sma_body_w, sma_body_w, sma_body_l - 0.01], center = true);
      color("gold") translate([0, 0, sma_body_l]) cylinder(d = sma_thread_d, h = sma_l - sma_body_l);
    }
    // straight antenna on top of the lid
    if (show_antenna) color("black") translate([sma_x, cc_cy, in_h + lid_t + lid_dz]) cylinder(d = 10, h = 55);
    // OLED: PCB + glass, flex loop on the front edge, 4-pin header along the back
    // edge pointing down, jumpers
    if (show_oled) translate([oled_x, oled_y, in_h + lid_dz]) {
      color("white") translate([0, 0, -oled_t/2]) cube([oled_pcb_w, oled_pcb_h, oled_t], center = true);
      color("orange") translate([-6, -oled_pcb_h/2 - oled_fpc, -oled_t]) cube([12, oled_fpc, oled_t]);
      translate([-2*2.54, oled_hdr_y - oled_y - 1.27, -oled_t]) {
        color("black") translate([0, 0, -hdr_spacer]) cube([4*2.54, 2.54, hdr_spacer]);
        color("dimgray") translate([0, 0, -hdr_spacer - dupont_l]) cube([4*2.54, 2.54, dupont_l]);
      }
    }
    // tactile switch on its pillar: body, legs, stem, cap
    if (show_button) translate([btn_x, btn_y, btn_seat]) {
      color("black") translate([-btn_w/2, -btn_w/2, 0]) cube([btn_w, btn_w, btn_body_h]);
      color("silver") for (i = [-1, 1], j = [-1, 1])
        translate([i*6.25 - 0.35, j*2.5 - 0.5, -3.5]) cube([0.7, 1, 3.5]);
      color("green") translate([0, 0, btn_body_h]) cylinder(d = 4, h = 1.5);
      color("green") translate([0, 0, btn_body_h + 1.5]) cylinder(d = btn_cap_d, h = btn_top - btn_body_h - 1.5);
    }
  }
}

// ---------- output ----------
module scene() {
  if (show_box) box();
  if (show_lid) color("orange", lid_opacity) translate([0, 0, lid_lift]) lid();
  parts(lid_dz = lid_lift);
}

if (part == "box") box();
else if (part == "lid") lid_print();
else if (part == "test") {
  translate([0, 0, -(oz - 6)]) intersection() {
    box();
    translate([-1, -1, oz - 6]) cube([ox + 2, oy + 2, 10]);
  }
  translate([ox + 5, 0, 0]) lid_print();
}
else if (cut_y > 0) intersection() {  // the cut view loses the colours
  scene();
  translate([-1, wall + cut_y, -1]) cube([ox + 2, oy + 2, 200]);
}
else scene();

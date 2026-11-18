#!/bin/sh
# Regenerates the STLs (hardware/stl/, gitignored) and the README images (docs/enclosure*.png)
# from enclosure.scad. Usage, from anywhere: sh hardware/render.sh
set -e
cd "$(dirname "$0")/.."
SCAD=hardware/enclosure.scad

mkdir -p hardware/stl
for part in box lid test; do
  openscad --backend=manifold -D "part=\"$part\"" -o "hardware/stl/$part.stl" "$SCAD"
done

img() {
  out=$1; shift
  openscad --imgsize=1200,1000 --colorscheme=Tomorrow "$@" -o "docs/$out.png" "$SCAD"
}
img enclosure         --camera=33,30,45,58,0,25,330 -D lid_lift=0 -D lid_opacity=1
img enclosure-open    --camera=33,30,50,55,0,25,380 -D lid_lift=35
img enclosure-top     --projection=ortho --camera=33,30,20,0,0,0,160 -D lid_lift=0 -D lid_opacity=1 -D show_antenna=false
img enclosure-usb     --camera=33,30,22,72,0,115,250 -D lid_lift=0 -D lid_opacity=1 -D show_antenna=false
img enclosure-inside  --camera=33,30,20,45,0,330,230 -D show_lid=false -D show_antenna=false
img enclosure-section --projection=ortho --camera=33,30,22,90,0,0,170 -D lid_lift=0 -D cut_y=44 -D show_antenna=false

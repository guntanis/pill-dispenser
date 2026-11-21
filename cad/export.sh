#!/usr/bin/env bash
# Export printable parts to stl/. Override binary with: OPENSCAD=/path/to/openscad ./export.sh
# macOS fallback: /Applications/OpenSCAD.app/Contents/MacOS/OpenSCAD
set -e
cd "$(dirname "$0")"
if [ -z "$OPENSCAD" ]; then
  if command -v openscad >/dev/null 2>&1; then
    OPENSCAD=openscad
  else
    OPENSCAD=/Applications/OpenSCAD.app/Contents/MacOS/OpenSCAD
  fi
fi
mkdir -p stl
for p in disk base_plate hopper hopper_lid chute; do
  echo "Exporting $p"
  "$OPENSCAD" -o "stl/$p.stl" -D 'part="'$p'"' assembly.scad
done

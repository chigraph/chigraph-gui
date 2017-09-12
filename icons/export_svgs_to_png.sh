#!/bin/bash

set -xe


imgdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

mkdir -p 16x16 32x32 64x64 128x128 256x256 512x512 1024x1024

ink=`which inkscape`
$ink chigraphicon.svg -w 16 -h 16 -e 16x16/org.chigraph.chigraphgui.png
$ink chigraphicon.svg -w 32 -h 32 -e 32x32/org.chigraph.chigraphgui.png
$ink chigraphicon.svg -w 64 -h 64 -e 64x64/org.chigraph.chigraphgui.png
$ink chigraphicon.svg -w 128 -h 128 -e 128x128/org.chigraph.chigraphgui.png
$ink chigraphicon.svg -w 256 -h 256 -e 256x256/org.chigraph.chigraphgui.png
$ink chigraphicon.svg -w 256 -h 256 -e 512x512/org.chigraph.chigraphgui.png
$ink chigraphicon.svg -w 1024 -h 1024 -e 1024x1024/org.chigraph.chigraphgui.png

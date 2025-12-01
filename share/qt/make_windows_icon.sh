#!/bin/bash
# create multiresolution windows icon
ICON_SRC=../../src/qt/res/icons/raqcoin.png
ICON_DST=../../src/qt/res/icons/raqcoin.ico
convert ${ICON_SRC} -resize 16x16 raqcoin-16.png
convert ${ICON_SRC} -resize 32x32 raqcoin-32.png
convert ${ICON_SRC} -resize 48x48 raqcoin-48.png
convert raqcoin-16.png raqcoin-32.png raqcoin-48.png ${ICON_DST}


#!/bin/bash

SRC_PNG=Casper2020\@3x.png
ICONSET=src/casper/mac/resources/casper.iconset

mkdir -p $ICONSET

sips -z 18 18     $SRC_PNG --out $ICONSET/icon_18x18.png
sips -z 36 36     $SRC_PNG --out $ICONSET/icon_18x18\@2.png
sips -z 54 54     $SRC_PNG --out $ICONSET/icon_18x18\@3.png

sips -z 16 16     $SRC_PNG --out $ICONSET/icon_16x16.png
sips -z 32 32     $SRC_PNG --out $ICONSET/icon_16x16\@2x.png
sips -z 32 32     $SRC_PNG --out $ICONSET/icon_32x32.png
sips -z 64 64     $SRC_PNG --out $ICONSET/icon_32x32\@2x.png
sips -z 128 128   $SRC_PNG --out $ICONSET/icon_128x128.png
sips -z 256 256   $SRC_PNG --out $ICONSET/icon_128x128\@2x.png
sips -z 256 256   $SRC_PNG --out $ICONSET/icon_256x256.png
sips -z 512 512   $SRC_PNG --out $ICONSET/icon_256x256\@2x.png
sips -z 512 512   $SRC_PNG --out $ICONSET/icon_512x512.png
sips -z 512 512   $SRC_PNG --out $ICONSET/icon_512x512\@2x.png
iconutil -c icns $ICONSET

rm -R $ICONSET
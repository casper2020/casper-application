#!/bin/bash

set -e

PWD=`pwd`

# URL=http://opensource.spotify.com/cefbuilds
# TAR_BZ2=cef_binary_3.3538.1849.g458cc98_macosx64.tar.bz2

ARCH=`uname -m`

URL=https://cef-builds.spotifycdn.com
if [ 'arm64' == "$ARCH" ] ; then
    TAR_BZ2=cef_binary_99.2.6+gf1f3fc8+chromium-99.0.4844.51_macosarm64.tar.bz2
else
    TAR_BZ2=cef_binary_99.2.7+g674fc01+chromium-99.0.4844.51_macosx64.tar.bz2
fi

echo "${URL}/${TAR_BZ2}"

THIRD_PARTY_DIR=$PWD/third-party/cef
TAR_BZ2_FILE=$THIRD_PARTY_DIR/$TAR_BZ2

__dir=`dirname $TAR_BZ2_FILE`
__file=`basename -- $TAR_BZ2_FILE`
__ext_dir=$__dir/${__file/.tar.bz2/}
__frameworks_dir=$__dir/Frameworks

if [ ! -f $TAR_BZ2_FILE ] ; then
    mkdir -p $__dir
    curl -o $TAR_BZ2_FILE $URL/$__file
    pushd $__dir
    tar xvjf $__file
fi

pushd $__ext_dir
cmake -G "Xcode" -DPROJECT_ARCH="${ARCH}"
popd

mkdir -p $__frameworks_dir
pushd $__frameworks_dir
ln -sf $__ext_dir/Release .
ln -sf $__ext_dir/Debug .
popd

ln -sf $__ext_dir/libcef_dll
ln -sf $__ext_dir/include
ln -sf $__ext_dir/cef.xcodeproj .

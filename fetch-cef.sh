#!/bin/bash

set -e

PWD=`pwd`
TAR_BZ2=cef_binary_3.3538.1849.g458cc98_macosx64.tar.bz2
THIRD_PARTY_DIR=$PWD/third-party/cef
TAR_BZ2_FILE=$THIRD_PARTY_DIR/$TAR_BZ2

if [ ! -f $TAR_BZ2_FILE ] ; then
    __dir=`dirname $TAR_BZ2_FILE`
    __file=`basename -- $TAR_BZ2_FILE`
    __ext_dir=$__dir/${__file/.tar.bz2/}
    __frameworks_dir=$__dir/Frameworks
    mkdir -p $__dir
    curl -o $TAR_BZ2_FILE http://opensource.spotify.com/cefbuilds/$__file
    pushd $__dir
    tar xvjf $__file

    pushd $__ext_dir
    cmake -G "Xcode" -DPROJECT_ARCH="x86_64"
    popd

    mkdir -p $__frameworks_dir
    pushd $__frameworks_dir
    ln -sf $__ext_dir/Release .
    ln -sf $__ext_dir/Debug .
    popd
    ln -sf $__ext_dir/libcef_dll
    ln -sf $__ext_dir/include
    popd
fi

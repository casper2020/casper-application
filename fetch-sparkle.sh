#!/bin/bash

set -e
set -x
PWD=`pwd`

NAME=Sparkle
VERSION=1.21.2

THIRD_PARTY_DIR=$PWD/third-party/Sparkle

TAR_BZ2=$NAME-${VERSION}.tar.bz2
TAR_BZ2_URL=https://github.com/sparkle-project/$NAME/releases/download/${VERSION}/$TAR_BZ2
TAR_BZ2_FILE=$THIRD_PARTY_DIR/$TAR_BZ2

if [ ! -f $TAR_BZ2_FILE ] ; then
    __dir=`dirname $TAR_BZ2_FILE`
    __file=`basename -- $TAR_BZ2_FILE`
    __ext_dir=$__dir/${__file/.tar.bz2/}
    __frameworks_dir=$__dir/Frameworks
    mkdir -p $__dir
    curl -L -o $TAR_BZ2_FILE $TAR_BZ2_URL
    pushd $__dir
    tar xvjf $__file
    popd
fi

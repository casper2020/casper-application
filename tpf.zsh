#!/bin/zsh

set -e

ACTION="$@"

PWD=`pwd`

CEF_FRAMEWORK_DIR=${CASPER_APP_SRC_DIR}/../third-party/cef/Frameworks/${CONFIGURATION}/Chromium\ Embedded\ Framework.framework
CEF_FRAMEWORK_DIR=`${READLINK_BIN} -m ${CEF_FRAMEWORK_DIR}`

CEF_FRAMEWORK_LIB=${CASPER_APP_SRC_DIR}/../third-party/cef/Frameworks/${CONFIGURATION}/cef_sandbox.a

CASPER_APP_FRAMEWORKS_DIR=`${READLINK_BIN} -m ${CASPER_APP_SRC_DIR}/../Frameworks`

if [ ! -z "${ACTION}" ] && [ 'clean' = "${ACTION}" ]  ; then
    rm -rf ${CASPER_APP_FRAMEWORKS_DIR}
else
    if [ ! -d ${CASPER_APP_FRAMEWORKS_DIR} ] ; then
        mkdir -p ${CASPER_APP_FRAMEWORKS_DIR}
        cp -RfL "${CEF_FRAMEWORK_DIR}" ${CASPER_APP_FRAMEWORKS_DIR}
        cp -fL "${CEF_FRAMEWORK_LIB}" ${CASPER_APP_FRAMEWORKS_DIR}
    fi
fi

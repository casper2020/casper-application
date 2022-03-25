#!/bin/zsh

set -e

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

#
# for cef library loaded
#
if [ ! -z "${ACTION}" ] && [ 'clean' = "${ACTION}" ]  ; then
    : # pass
else
    HELPERS=('casper Helper' 'casper Helper (GPU)' 'casper Helper (Renderer)' 'casper Helper (Plugin)')
    for helper in ${HELPERS[@]} ; do
        build_dir=${NRS_PRODUCTS_DIR/tpf/${helper}}
        if [ ! -d ${build_dir} ] ; then
            continue;
        fi
        build_dir=${NRS_PRODUCTS_DIR/tpf/}
        mkdir -p ${build_dir}
        pushd ${build_dir}
        ln -sf "./casper.app/Contents/Frameworks/Chromium Embedded Framework.framework" 'Chromium Embedded Framework.framework'
        popd
        break
    done
fi

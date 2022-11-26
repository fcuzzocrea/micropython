#!/bin/sh
# run this from the root directory of the micropython-leon repository

if [ $# -ne 1 ]; then
    echo "usage: $0 <revision-id>"
    exit 1
fi

REV=$1

if [ ! -d leon-common ]; then
    echo "not in the MicroPython LEON directory"
    exit 1
fi

if [ -r VERSION ]; then
    echo "VERSION file should not exist"
    exit 1
fi

echo -e "MicroPython port to SPARC/LEON/RTEMS platforms\nCopyright (c) 2015-2018 George Robotics Limited\nVersion $REV\nPackaged on $(date -u)\nSource repository tag $(git describe)" > VERSION

/bin/ln -s . micropython-leon-$REV

tar c \
    --exclude='build*' --exclude="*.srec" --exclude=".gitignore" --exclude=".gitattributes" \
    --exclude="mpy-cross/mpy-cross" --exclude="mpy-cross/mpy-cross.map" --exclude="*.mpy" --exclude="__pycache__" \
    micropython-leon-$REV/{VERSION,make-all.sh,BUILD_GUIDE,CODING_GUIDE,TEST_RESULTS,py,lib/libc,lib/utils,lib/micropython-ulab,extmod/*.[ch],extmod/crypto-algorithms,extmod/uzlib,tools/cc1,docs/conf.py,mpy-cross,leon-common,leon-ex-minimal,leon-ex-tasks,leon-ex-manager,leon-ex-pystone,leon-for-tests,leon-tests,leon-obcp,leon-spacebel} | gzip > GRL_MPLEON_SWPKG_$REV.tgz

/bin/rm VERSION
/bin/rm micropython-leon-$REV
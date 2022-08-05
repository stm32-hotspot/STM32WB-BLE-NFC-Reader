#!/bin/bash

GIT_ROOT=../..

OUTFILENAME=ndef_sizes.txt
OUTFILEPATH=ndef/doc/doxyfiles

TARGET=X-NUCLEO-NFC06A1

#cat ${GIT_ROOT}/nucleo/Firmware/nucleo/${TARGET}/STM32/MDK-ARM/${TARGET}-NDEF/${TARGET}-NDEF.map | sed -n -e '/Image component sizes/,$p' | tee $OUTFILE

echo "Make sure the library is compiled with -O2 optimization level"

for f in ${GIT_ROOT}/ndef/source/poller/*.c ${GIT_ROOT}/ndef/source/message/*.c ; do
OBJS+="`basename $f c`o "
done
#echo $OBJS

cd ${GIT_ROOT}/nucleo/Firmware/nucleo/${TARGET}/STM32/MDK-ARM/${TARGET}-NDEF
size --total $OBJS > ${OUTFILENAME}
cd -

mv ${GIT_ROOT}/nucleo/Firmware/nucleo/${TARGET}/STM32/MDK-ARM/${TARGET}-NDEF/${OUTFILENAME} ${GIT_ROOT}/${OUTFILEPATH}/${OUTFILENAME}

echo Created ${GIT_ROOT}/${OUTFILEPATH}/${OUTFILENAME}:
cat ${GIT_ROOT}/${OUTFILEPATH}/${OUTFILENAME}

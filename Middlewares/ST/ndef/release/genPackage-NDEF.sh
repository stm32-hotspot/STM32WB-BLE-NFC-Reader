#!/bin/bash

# Script to create the NDEF library release

RFAL_LIB=st25r3916
RFAL_VERSION=v2.0.10

NDEF_VERSION=v1.0.4

OUTPUT_DIR=RFAL_${RFAL_LIB}_${RFAL_VERSION}_NDEF_${NDEF_VERSION}

XNUCLEO_VERSION=NFC06A1

EXCLUDE=--exclude=.git*

RFAL_EXCLUDE="--exclude=st25r3911 --exclude=st25r95 --exclude=sts39230"

NDEF_EXCLUDE="--exclude=release --exclude=doxyfiles --exclude=test"

NUCLEO_EXCLUDE="--exclude=STM8 --exclude=prqa --exclude=BSP --exclude=DSP_Lib --exclude=STM32L4xx/Source --exclude=CMSIS/Include --exclude=RTOS --exclude=Unittests --exclude=MDK-ARM --exclude=.mxproject --exclude=prqaproject.xml --exclude=main_tests.c"

GIT_ROOT=../../

#----------------------------------------------------------------------

# Check git status
if [[ `git status --porcelain` ]]; then
    #echo "Can't create the release with modified files in the git repository. Exit."; exit
	echo "!!! WARNING: modified files in the git repository - NOT SAFE TO CREATE THE RELEASE !!!"
fi

echo "Package name:" ${OUTPUT_DIR}

#----------------------------------------------------------------------
#if false; then  # Skip package content creation, jump to archive creation

# Delete and Create Output folder
if [ -d ${OUTPUT_DIR} ]
then
    echo "Delete existing output directory"
	rm -rf ${OUTPUT_DIR}
fi

echo "Create Output folder:" ${OUTPUT_DIR}
mkdir ${OUTPUT_DIR}


#----------------------------------------------------------------------
# Generate binary file size
#./generate_ndef_sizes.bash

#----------------------------------------------------------------------
# Generate NDEF .chm
cd ../doc/doxyfiles
doxygen.exe
cp ../doxygen/html/ndef.chm ..
rm -rf ../doxygen
cd -

#----------------------------------------------------------------------
# Common
echo "Copying files from the Common folder"
COMMON_DIR=common/firmware/STM

COMMON_DIR_UTILS=${COMMON_DIR}/utils/Inc
mkdir -p ${OUTPUT_DIR}/${COMMON_DIR_UTILS}
rsync ${GIT_ROOT}/${COMMON_DIR_UTILS}/st_errno.h ${OUTPUT_DIR}/${COMMON_DIR_UTILS}/
rsync ${GIT_ROOT}/${COMMON_DIR_UTILS}/utils.h    ${OUTPUT_DIR}/${COMMON_DIR_UTILS}/

COMMON_DIR_INC=${COMMON_DIR}/STM32/Inc
mkdir -p ${OUTPUT_DIR}/${COMMON_DIR_INC}
rsync ${GIT_ROOT}/${COMMON_DIR_INC}/spi.h   ${OUTPUT_DIR}/${COMMON_DIR_INC}/
rsync ${GIT_ROOT}/${COMMON_DIR_INC}/timer.h ${OUTPUT_DIR}/${COMMON_DIR_INC}/

COMMON_DIR_SRC=${COMMON_DIR}/STM32/Src
mkdir -p ${OUTPUT_DIR}/${COMMON_DIR_SRC}
rsync ${GIT_ROOT}/${COMMON_DIR_SRC}/spi.c   ${OUTPUT_DIR}/${COMMON_DIR_SRC}/
rsync ${GIT_ROOT}/${COMMON_DIR_SRC}/timer.c ${OUTPUT_DIR}/${COMMON_DIR_SRC}/

#----------------------------------------------------------------------
# RFAL
echo "Copying the RFAL folder"
RFAL_DIR=rfal
mkdir -p ${OUTPUT_DIR}/${RFAL_DIR}
rsync -r ${EXCLUDE} ${RFAL_EXCLUDE} ${GIT_ROOT}/${RFAL_DIR}    ${OUTPUT_DIR}

#----------------------------------------------------------------------
# NDEF Library
echo "Copying the NDEF folder"
NDEF_DIR=ndef
mkdir -p ${OUTPUT_DIR}/${NDEF_DIR}
rsync -r ${EXCLUDE} ${NDEF_EXCLUDE} ${GIT_ROOT}/${NDEF_DIR}    ${OUTPUT_DIR}

#----------------------------------------------------------------------
# Nucleo
echo "Copying the Nucleo folder"
NUCLEO_DIR=nucleo
mkdir -p ${OUTPUT_DIR}/${NUCLEO_DIR}

DEMO_INC=${NUCLEO_DIR}/Firmware/nucleo/Inc
mkdir -p ${OUTPUT_DIR}/${DEMO_INC}
rsync -r ${EXCLUDE} ${GIT_ROOT}/${DEMO_INC}/    ${OUTPUT_DIR}/${DEMO_INC}

DEMO_SRC=${NUCLEO_DIR}/Firmware/nucleo/Src
mkdir -p ${OUTPUT_DIR}/${DEMO_SRC}
rsync -r ${EXCLUDE} ${GIT_ROOT}/${DEMO_SRC}/    ${OUTPUT_DIR}/${DEMO_SRC}

DEMO_PLATFORM=${NUCLEO_DIR}/Firmware/nucleo/X-NUCLEO-${XNUCLEO_VERSION}
mkdir -p ${OUTPUT_DIR}/${DEMO_PLATFORM}
rsync -r ${EXCLUDE} ${NUCLEO_EXCLUDE} ${GIT_ROOT}/${DEMO_PLATFORM}/    ${OUTPUT_DIR}/${DEMO_PLATFORM}

DEMO_PROJECT=${NUCLEO_DIR}/Firmware/nucleo/X-NUCLEO-${XNUCLEO_VERSION}/STM32/MDK-ARM
mkdir -p ${OUTPUT_DIR}/${DEMO_PROJECT}
rsync -r ${EXCLUDE} ${GIT_ROOT}/${DEMO_PROJECT}/startup_stm32l476xx.s                    ${OUTPUT_DIR}/${DEMO_PROJECT}
# .uvprojx contains 3 targets: the default + NDEF + NDEF-Example target
rsync -r ${EXCLUDE} ${GIT_ROOT}/${DEMO_PROJECT}/X-NUCLEO-${XNUCLEO_VERSION}.uvprojx                 ${OUTPUT_DIR}/${DEMO_PROJECT}

echo -e "\nRelease content generated"


#----------------------------------------------------------------------
# Archive
#fi
#echo "Creating archive ${OUTPUT_DIR}.tar.gz"
#tar -czf ${OUTPUT_DIR}.tar.gz ${OUTPUT_DIR}
# Zip the package
echo "Zipping..."
/c/Program\ Files/7-Zip/7z a ${OUTPUT_DIR}.zip ${OUTPUT_DIR}

echo -e "\nPress any key to quit"
read

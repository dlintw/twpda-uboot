#!/bin/bash
set -o errexit

# use make.sh <uboot_version> # if fossil not exist
UBOOT_VER=${1:-1}
if [ "$UBOOT_VER" == "1" -a -x /usr/bin/fossil -a -r manifest.uuid ] ; then
	# use fossil version
	_id=$(cat manifest.uuid 2>/dev/null)
	UBOOT_VER=$(printf %u 0x${_id:0:6})
	echo UBOOT_VER="$UBOOT_VER"
	echo "#define CURRENTSVNVERSION \"$UBOOT_VER\"" \
		> src/include/SvnVersion.h
fi
if [ ! -r src/include/SvnVersion.h ] ; then
	echo "#define CURRENTSVNVERSION \"exported\"" \
		> src/include/SvnVersion.h
fi

jobs=`grep processor  /proc/cpuinfo |wc -l`
jobs=$((jobs+1))
echo "jobs=$jobs"

cd src
make pdk7105se_config
CFLAGS="-Wall -Wformat" make -j$jobs
cp u-boot.bin ../uboot_update_tool
cd ../uboot_update_tool
chmod +x MakeBinFile
chmod +x makeupdateSW.sh
UPDATEUBOOT=y
HW_VER=1
DEV_ID=620
FAC_ID=101
DATE=$(date +%y%m)
rm -f $PWD/iptvubootupdate.bin
echo "UBOOT_VER=$UBOOT_VER"
./MakeBinFile $UPDATEUBOOT $UBOOT_VER $HW_VER $DEV_ID $FAC_ID $DATE
ls -l $PWD/iptvubootupdate.bin

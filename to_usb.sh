#!/bin/sh
set -o errexit
pmount /dev/sdb1
cp uboot_update_tool/iptvubootupdate.bin /media/sdb1
pumount /dev/sdb1

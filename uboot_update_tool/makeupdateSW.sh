#!/bin/sh

echo ""
echo "**********"**********"**********"**********"*****"
echo "******* Welcom to use Make Update File (for GB620) ******"
echo "**********"**********"**********"**********"*****"

echo ""
echo "-->Need Update uboot? Set \"y\" to update,set \"n\" to ignore"
read -p "UPDATEUBOOT =" UPDATEUBOOT
if [ "$UPDATEUBOOT" = y ]
	then
		echo ""
		echo "-->Please input uboot SW version:"
		read -p "UBOOT_VER =" UBOOT_VER
else
		UPDATEUBOOT=n
		UBOOT_VER=0
fi


echo ""
echo "-->Please input HW version:"
echo "SMIT: input \"1\" "
read -p "HW_VER =" HW_VER

echo ""
echo "-->Please input device ID: "
echo "GB620: input \"620\" "
read -p "DEV_ID =" DEV_ID

echo ""
echo "-->Please input factory ID:"
echo "SMIT: input \"101\" "
read -p "FAC_ID =" FAC_ID

echo ""
echo "-->Please input Date:"
echo "eg: 2010-05  input \"1005\" "
read -p "DATE =" DATE


echo ""
echo ""
echo "**************** Confirm Your Input Information *************"
echo ""
echo "Update uboot: $UPDATEUBOOT"
if [ "$UBOOT_VER" != 0 ]
	then
echo "Uboot version: $UBOOT_VER"
fi

echo "SW_VER: $SW_VER"
echo "HW_VER: $HW_VER"
echo "DEV_ID: $DEV_ID"
echo "FAC_ID: $FAC_ID"
echo "Date  : $DATE"



echo ""
echo "****************Please Confirm above Information *************"
echo ""

echo "If the input info is no error,please input \"y\", or input \"n\"."
read -p "InputInfoNoError =" InputInfoNoError


if [ "$InputInfoNoError" = y ]
	then
	echo "Call MakeBinFile:"
./MakeBinFile  $UPDATEUBOOT $UBOOT_VER $HW_VER $DEV_ID $FAC_ID $DATE  

else
	echo "Please retry!!!"
fi


# uboot.sh http://www.stlinux.com/u-boot/using

# uncomment following line to force use external usb
# usbcfg 1 ; usb start ; usb tree

# uncomment following line to allow press key to stop boot
# setenv bootdelay 10

# boot parameters for external USB partition 2

# sda1 8:1 sda2 8:2 sdb1 8:17 sdb2 8:18
setenv bootargs 'console=ttyAS0,115200 rootdelay=0 root=8:18 rootfstype=ext3 rw rootflags=data=journal nwhwconf=device:eth0,hwaddr:10:08:E2:12:06:BD phyaddr:0,watchdog:5000 mem=120M bigphysarea=2048'

# 0:1 sda1 0:2 sda2 1:1 sda2 1:2 sda2
setenv bootcmd 'ext2load usb 0:2 80000000 vmlinux.ub; bootm 80000000'

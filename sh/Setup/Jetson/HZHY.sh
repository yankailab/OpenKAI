# install nvidia SDK manager

# if using Ubuntu later than 18.04, fake it with
sudo nano /etc/lsb-release
# with the following content
DISTRIB_ID=Ubuntu
DISTRIB_RELEASE=18.04
DISTRIB_CODENAME=bionic
DISTRIB_DESCRIPTION=“Ubuntu 18.04.4 LTS”
# run sdkmanager

# download JetPack 4.6 rev2 withOUT flashing Jetson
# JetPack 4.6 rev3 may also work but not tested yet

# change this to the same Linux_for_Tegra folder on your computer
export DIRL4T=/home/kai/nvidia/nvidia_sdk/JetPack_4.6_Linux_JETSON_XAVIER_NX_TARGETS/Linux_for_Tegra

# sync the source code
cd $DIRL4T
./source_sync.sh

# find the correspondent tags
cd $DIRL4T/sources/kernel/kernel-4.9
git tag -l tegra-l4t-r32.6.1
# Jetson Xavier NX (Jetpack 4.6): tegra-l4t-r32.6.1
# Jetson Nano (Jetpack 4.5.1): tegra-l4t-r32.5.1

# patching the kernel
cp sources-AI300serial2-nx $DIRL4T/sources/
sudo patch -p1 < sources-AI300serial2-nx

# build the system image
sudo apt-get install aarch64-linux-gnu-gcc
cd $DIRL4T/sources/kernel/kernel-4.9

# generate config file
sudo make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- LOCALVERSION=-tegra tegra_defconfig

# build kernel DTB and modules
sudo make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- LOCALVERSION=-tegra -j8

# install kernel modules
sudo make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- LOCALVERSION=-tegra modules_install INSTALL_MOD_PATH=../../../rootfs/

# copy the files built to image
cp arch/arm64/boot/Image ../../../kernel/Image
cp arch/arm64/boot/dts/*.dtb ../../../kernel/dtb/

# override modules
cd $DIRL4T/rootfs
sudo tar --owner root --group root -cjf kernel_supplements.tbz2 lib/modules
cp kernel_supplements.tbz2 $DIRL4T/kernel/

cd $DIRL4T
sudo ./apply_binaries.sh

# Connect your Jetson board USB type-C port to host PC USB
# Put Jetson into recovery mode by keep pushing the dip button on the back of the board while power on
# make sure the Jetson is connected in Recovery mode by lsusb command and find if there is NVIDIA in it
cd $DIRL4T

# Jetson Xavier NX(P3688-0001 module)
sudo ./flash.sh p3449-0000+p3668-0001-qspi-emmc mmcblk0p1
# Jetson Xavier NX [developer kit version](P3668-0000 module)
sudo ./flash.sh jetson-xavier-nx-devkit mmcblk0p1
# Jetson Nano(P3448-0002 module)
sudo ./flash.sh jetson-nano-emmc mmcblk0p1
# Jetson Nano [developer kit version](P3448-0000 module)
sudo ./flash.sh jetson-nano-devkit mmcblk0p1

# Now the Jetson should boot from eMMC,
# connect a monitor with HDMI to your Jetson, when the desktop comes out, finish the setup on Jetson

# ssh using USB LAN
ssh lab@192.158.55.1

# Following is optional only if you need to boot from SD card

# Preparing SD card

# insert SD card into your host PC, launch ubuntu Disks.

# Click the SD card drive on the left column, in the right pane, click the three bar-ed button on top right corner, then "Format disk", choose partition table type to be "empty" form the pull down selection.

# on host PC,
sudo apt-get install Gparted

# launch GParted, select /dev/sdb (the one represents SD card) from top right

# click the main bar with label "/dev/sdb", then click menu "Device" -> "Create Partition table",

# create the table with "Linux ext4" with "Primary partition" type, then make sure to click the "v" button on the menu bar to apply the operation.

# on host PC,
sudo fdisk -l
#check if there is a /dev/sdb1 partition under /dev/sdb

# insert the SD card into Jetson
# on Jetson, run sudo fdisk -l, chcke if there is a /dev/mmcblk1p1 partition under /dev/mmcblk1

# copy system to SD card
df -h
sudo umount /dev/mmcblk1p1
#sudo mkfs.ext4 /dev/mmcblk1p1
sudo ./copy-rootfs-tf.sh
sudo ./setup-service.sh
sync
sudo reboot now
df -h

# Clone
# flash an new Jetson unit and complete the setup, camera settings
# clone the SD card and insert to another unit, then run
sudo mount /dev/mmcblk1p1 /mnt
sudo ./setup-service.sh
sync
sudo reboot now
df -h

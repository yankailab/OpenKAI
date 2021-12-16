# install nvidia SDK manager

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

# copy system to SD card, insert a formated SD card on Jetson
df -h
sudo umount /dev/mmcblk1p1
sudo mkfs.ext4 /dev/mmcblk1p1
sudo ./copy-rootfs-tf.sh
sudo ./setup-service.sh
sync
sudo reboot now
df -h

# Flash the new Jetson unit and complete the setup, camera settings

# clone the SD card and insert to another unit, then run
sudo ./setup-service.sh
sync
sudo reboot now
df -h


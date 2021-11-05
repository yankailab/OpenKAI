# install JetPack without flashing Jetson

export DIRL4T=/home/kai/nvidia/nvidia_sdk/JetPack_4.6_Linux_JETSON_XAVIER_NX_TARGETS/Linux_for_Tegra

# sync the source code
cd $DIRL4T
./source_sync.sh
# find the correspondent tags
cd $DIRL4T/sources/kernel/kernel-4.9
git tag –l tegra-l4t*
# Jetson Nano (Jetpack 4.5.1): tegra-l4t-r32.5.1
# Jetson Xavier NX (Jetpack 4.6): tegra-l4t-r32.6.1

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

# burn image, put Jetson into recovery mode
cd $DIRL4T
# Jetson Xavier NX(P3688-0001 module)
sudo ./flash.sh p3449-0000+p3668-0001-qspi-emmc mmcblk0p1
# Jetson Xavier NX [developer kit version](P3668-0000 module)
sudo ./flash.sh jetson-xavier-nx-devkit mmcblk0p1
# Jetson Nano(P3448-0002 module)
sudo ./flash.sh jetson-nano-emmc mmcblk0p1
# Jetson Nano [developer kit version](P3448-0000 module)
sudo ./flash.sh jetson-nano-devkit mmcblk0p1

# copy system to SD card, insert a formated SD card on Jetson
df -h
sudo umount /dev/mmcblk1p1
sudo mkfs.ext4 /dev/mmcblk1p1
sudo ./copy-rootfs-tf.sh
sudo ./setup-service.sh
sync
sudo reboot now
df -h
# clone the SD card and insert to another unit, then run
sudo ./setup-service.sh
sync
sudo reboot now
df -h


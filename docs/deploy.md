# Make boot SD image

```bash
# clone SD image
sudo fdisk -l
sudo umount /dev/sdb1 /dev/sdb2
sudo umount /dev/sdb
sudo dd if=/dev/sdb of=~/RPi.img bs=8M status=progress conv=fsync

# restore to any larger card:
sudo dd if=RPi.img of=/dev/sdb bs=8M status=progress conv=fsync
```

Using PiShrink
```bash
sudo apt-get update
sudo apt-get install wget parted gzip pigz xz-utils udev e2fsprogs
wget https://raw.githubusercontent.com/Drewsif/PiShrink/master/pishrink.sh
chmod +x pishrink.sh
sudo mv pishrink.sh /usr/local/bin

sudo pishrink.sh RPi.img RPi_shrunk.img

# restore to any larger card:
sudo dd if=RPi_shrunk.img of=/dev/sdb bs=8M status=progress conv=fsync
```

Usage: pishrink.sh [-adhnrsvzZ] imagefile.img [newimagefile.img]

  -s         Don't expand filesystem when image is booted the first time
  -v         Be verbose
  -n         Disable automatic update checking
  -r         Use advanced filesystem repair option if the normal one fails
  -z         Compress image after shrinking with gzip
  -Z         Compress image after shrinking with xz
  -a         Compress image in parallel using multiple cores
  -d         Write debug messages in a debug log file


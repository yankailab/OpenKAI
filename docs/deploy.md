# Make boot SD image
```bash
# clone SD image
sudo fdisk -l
sudo umount /dev/sdb1 /dev/sdb2
sudo umount /dev/sdb
sudo dd if=/dev/sdb of=~/RPi.img bs=8M status=progress conv=fsync

sudo pishrink.sh RPi.img RPi_shrunk.img

# restore to any larger card:
sudo dd if=RPi_shrunk.img of=/dev/sdB bs=8M status=progress conv=fsync
```

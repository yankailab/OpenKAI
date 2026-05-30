# Make boot SD image
```bash
sudo fdisk -l
sudo umount /dev/sda
sudo dd if=/dev/sda of=~/sd.img bs=6M

sudo fdisk -l
sudo umount /dev/sdb
sudo dd if=~/sd.img of=/dev/sdb bs=6M


# clone SD image
sudo fdisk -l
sudo umount /dev/sdb1 /dev/sdb2
sudo dd if=/dev/sdb of=~/RPi.img bs=8M status=progress
```


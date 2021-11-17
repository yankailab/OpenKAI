#!/bin/bash
sudo parted /dev/nvme0n1 mklabel gpt
sudo parted /dev/nvme0n1 mkpart logical 0 100%
sudo mkfs.ext4 /dev/nvme0n1p1

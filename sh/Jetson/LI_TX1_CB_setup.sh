# Flash JetPack3.2.1
# Reboot TX2 into "reset recovery mode" 

cp $HOME/dev/LI_TX1_CB/binaries/tegra186-quill-p3310-1000-c03-00-base.dtb $HOME/dev/JetPack3.2.1/64_TX2/Linux_for_Tegra/kernel/dtb
cd $HOME/dev/JetPack3.2.1/64_TX2/Linux_for_Tegra
sudo ./flash.sh -r -k kernel-dtb jetson-tx2 mmcblk0p1

# Reboot TX2 normally
scp -r $HOME/dev/LI_TX1_CB ubuntu@192.168.1.100:/home/ubuntu/dev

# SSH into TX2
ssh ubuntu@192.168.1.100

cd $HOME/dev/LI_TX1_CB/binaries
sudo cp Image /boot
sudo cp zImage /boot
sudo cp 4.4.38-tegra-leopard.tgz /lib/modules
cd /lib/modules
sudo tar zxvf 4.4.38-tegra-leopard.tgz

cd $HOME/dev/LI_TX1_CB/binaries
sudo cp camera_overrides.isp /var/nvidia/nvcam/settings
sudo chmod 664 /var/nvidia/nvcam/settings/camera_overrides.isp
sudo chown root:root /var/nvidia/nvcam/settings/camera_overrides.isp
sudo reboot now

gst-launch-1.0 nvcamerasrc fpsRange="60.0 60.0" sensor-id=0 ! 'video/x-raw(memory:NVMM), width=(int)3840, height=(int)2160, format=(string)I420, framerate=(fraction)60/1' ! nvtee ! nvvidconv flip-method=2 ! 'video/x-raw, format=(string)I420' ! xvimagesink -e  

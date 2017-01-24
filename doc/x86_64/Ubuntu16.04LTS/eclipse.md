#Eclipse
tar xzvf eclipse
--launcher.GTK_version
2
--launcher.appendVmargs
-vmargs


sudo echo -e "/usr/local/cuda/lib64\n/home/ubuntu/src/jetson-inference/build/x86_64/lib" >> /etc/ld.so.conf
sudo ldconfig

#OpenKAI
mkdir workspace
git clone https://github.com/yankailab/OpenKAI.git



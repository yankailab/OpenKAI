gst-launch-1.0 v4l2src device=/dev/video0 ! videoflip method=0 ! video/x-raw,format=BGR,width=1280,height=720,framerate=30/1 ! videoconvert ! omxh264enc insert-sps-pps=true ! rtph264pay mtu=1400 config-interval=1 pt=96 ! udpsink host=224.1.1.1 port=5678 auto-multicast=true

gst-launch-1.0 libcamerasrc ! video/x-raw,format=RGB,width=1280,height=720,framerate=30/1 ! v4l2convert ! v4l2h264enc ! 'video/x-h264,level=(string)4' ! h264parse ! tee name=t t. ! queue ! matroskamux ! filesink location=/home/pi/video/test.mkv t. ! queue ! rtph264pay mtu=1400 config-interval=1 pt=96 ! udpsink host=192.168.1.245 port=5678 auto-multicast=false

gst-launch-1.0 libcamerasrc ! video/x-raw,format=RGB,width=1280,height=720,framerate=30/1 ! v4l2convert ! v4l2h264enc ! 'video/x-h264,level=(string)4' ! h264parse ! rtph264pay mtu=1400 config-interval=1 pt=96 ! udpsink host=192.168.1.245 port=5678 auto-multicast=false

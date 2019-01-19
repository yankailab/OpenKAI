gst-launch-1.0 udpsrc port=5678 multicast-group=224.1.1.1 ! application/x-rtp, payload=96 ! rtph264depay ! avdec_h264 ! videoconvert ! autovideosink sync=false async=false


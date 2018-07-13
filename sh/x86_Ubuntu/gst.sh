gst-launch-1.0 udpsrc port=5678 ! application/x-rtp, payload=96 ! rtph264depay ! avdec_h264 ! videoconvert ! autovideosink sync=false async=false


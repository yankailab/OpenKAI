/*
 * _UVC.h
 *
 *  Created on: Feb 15, 2025
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__UVC_H_
#define OpenKAI_src_Vision__UVC_H_

#include "_VisionBase.h"
#include <libuvc/libuvc.h>
#include <libusb-1.0/libusb.h>

///////////////////////////////////////////////////////
/* bmRequestType */
#define REQ_TYPE_SET 0x21
#define REQ_TYPE_GET 0xa1

/* bmRequest */
#define REQ_SET_CUR 0x01
#define REQ_GET_CUR 0x81
#define REQ_GET_LEN 0x85

/* wValue: CS ID */
#define XU_CS_ID_SYSTEM (unsigned short)0x0100
/* sub-function ID */
#define SYSTEM_DEVICE_INFO (unsigned char)0x01
#define SYSTEM_REBOOT (unsigned char)0x02
#define SYSTEM_RESET (unsigned char)0x03
#define SYSTEM_HARDWARE_SERVER (unsigned char)0x04
#define SYSTEM_LOCALTIME (unsigned char)0x05
#define SYSTEM_UPDATE_FIRMWARE (unsigned char)0x06
#define SYSTEM_DIAGNOSED_DATA (unsigned char)0x07
#define SYSTEM_UPDATE_STATE (unsigned char)0x08

#define XU_CS_ID_IMAGE (unsigned short)0x0200
#define IMAGE_BRIGHTNESS (unsigned char)0x01
#define IMAGE_CONTRAST (unsigned char)0x02
#define IMAGE_BACKGROUND_CORRECT (unsigned char)0x03
#define IMAGE_MANUAL_CORRECT (unsigned char)0x04
#define IMAGE_ENHANCEMENT (unsigned char)0x05
#define IMAGE_VIDEO_ADJUST (unsigned char)0x06

#define XU_CS_ID_THERMAL (unsigned short)0x0300
#define THERMAL_BASIC_PARAM (unsigned char)0x01
#define THERMAL_MODE (unsigned char)0x02
#define THERMAL_MODE_NORMAL 1
#define THERMAL_MODE_EXPERT 2

#define THERMAL_ALG_VERSION (unsigned char)0x04
#define THERMAL_STREAM_PARAM (unsigned char)0x05
#define STREAM_TYPE_YUV_ONLY 6
#define STREAM_TYPE_YUV_RAW 9
#define STREAM_TYPE_YUV_TEMP 8

#define THERMAL_CALIBRATION_FILE (unsigned char)0x0e
#define THERMAL_EXPERT_REGIONS (unsigned char)0x0f
#define THERMAL_EXPERT_CORRECTION_PARAM (unsigned char)0x10
#define THERMAL_EXPERT_CORRECTION_START (unsigned char)0x11
#define THERMAL_TEMP_RISE_CALIBRATION (unsigned char)0x12

#define XU_CS_ID_PROTOCOL_VER (unsigned short)0x0400
#define XU_CS_ID_COMMAND_SWITCH (unsigned short)0x0500
#define XU_CS_ID_ERROR_CODE (unsigned short)0x0600

#define UVC_FMT_ANY "any"
#define UVC_FMT_UNCOMPRESSED "uncompressed"
#define UVC_FMT_COMPRESSED "compressed"
#define UVC_FMT_YUYV "yuyv"
#define UVC_FMT_UYVY "uyvy"
#define UVC_FMT_RGB "rgb"
#define UVC_FMT_BGR "bgr"
#define UVC_FMT_MJPEG "mjpeg"
#define UVC_FMT_GRAY8 "gray8"

namespace kai
{

	class _UVC : public _VisionBase
	{
	public:
		_UVC();
		virtual ~_UVC();

		bool init(const json& j);
		bool start(void);
		bool open(void);
		void close(void);
		void draw(void *pFrame);

		static void sCbGetFrame(uvc_frame *pFrame, void *p)
		{
			NULL_(pFrame);
			NULL_(p);

			((_UVC *)p)->cbGetFrame(pFrame);
		};

	private:
		int USBgetLen(unsigned short wValue);
		int USBgetCur(unsigned short wValue, unsigned char *pData, unsigned short len);
		int USBgetErrno(void);
		int USBsetCur(unsigned short wValue, unsigned char *pData, unsigned short len);
		bool USBsetCurFunc(unsigned short csID, unsigned char subID);
		int USBsetCurData(unsigned short csID, unsigned char *pB);
		int USBgetCurData(unsigned short csID, unsigned char *pB);
		int USBctrlTransfer(unsigned char bmRequestType, unsigned char bRequest,
							unsigned short wValue, unsigned short wIndex,
							unsigned char *pData, uint16_t wLength, unsigned int timeout);

		bool getProtocolVersion(void);
		bool setStreamType(unsigned char type);
		bool getDeviceInfo(void);

		bool UVCopen(void);
		void UVCclose(void);
		bool UVCsetVideoMode(void);
		bool UVCstreamStart(void);
		void UVCstreamClose(void);
		void UVCstreamGetFrame(unsigned int tOut);
		void cbGetFrame(uvc_frame *pFrame);

		void update(void);
		static void *getUpdate(void *This)
		{
			((_UVC *)This)->update();
			return NULL;
		}

	protected:
		uvc_context_t *m_pCtx;
		uvc_device_t *m_pDev;
		uvc_device_handle_t *m_pHandleDev;
		uvc_stream_ctrl_t m_ctrl;
		uvc_stream_handle_t *m_pHandleStream;
		uvc_frame_t *m_pUVCframe;
		uvc_frame_callback_t *m_pCB; //(uvc_frame_t* frame, void* ptr)

		int m_uvcFPS;
		int m_uvcSize;
		int m_uvcOffset;
		int m_uvcLen;
		unsigned char *m_pFptr;

		int m_streamType;
		int m_vendorID;
		int m_productID;
		string m_SN;

		vFloat2 m_vRangeDraw;

		/*
		add 99-hik.rules to /etc/udev/rules.d/
		sudo nano /etc/udev/rules.d/99-hik.rules
		SUBSYSTEM=="usb", ATTRS{idVendor}=="2bdf", MODE:="0666"
		*/
	};
}
#endif

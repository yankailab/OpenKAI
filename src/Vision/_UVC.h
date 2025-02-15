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

		int init(void *pKiss);
		int start(void);
		bool open(void);
		void close(void);

		static void sCbGetFrame(uvc_frame *pFrame, void *p)
		{
			NULL_(pFrame);
			NULL_(p);

			((_UVC *)p)->cbGetFrame(pFrame);
		};

	private:
		bool UVCopen(void);
		void UVCclose(void);
		void UVCsetVideoMode(void);
		bool UVCstreamStart(void);
		void UVCstreamClose(void);
		void UVCstreamGetFrame(unsigned int tOut);
		void UVCctrlTransfer(void);
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

		int m_vendorID;
		int m_productID;
		string m_SN;

		/*
		add 99-hik.rules to /etc/udev/rules.d/
		SUBSYSTEM=="usb", ATTRS{idVendor}=="", MODE:="0666"
		*/
	};
}
#endif

/*
 * _UVC.cpp
 *
 *  Created on: Feb 15, 2025
 *      Author: yankai
 */

#include "_UVC.h"

namespace kai
{

	_UVC::_UVC()
	{
		m_type = vision_uvc;

		m_pCtx = nullptr;
		m_pDev = nullptr;
		m_pHandleDev = nullptr;
		m_pHandleStream = nullptr;
		m_pUVCframe = nullptr;
		m_pCB = nullptr;
		m_pFptr = nullptr;

		m_streamType = 2;
		m_vendorID = 0;
		m_productID = 0;
		m_SN = "";

		m_vRangeDraw.set(0, 100);
	}

	_UVC::~_UVC()
	{
		close();
	}

	bool _UVC::init(const json &j)
	{
		IF_F(!_VisionBase::init(j));

		jKv(j, "streamType", m_streamType);
		jKv(j, "vendorID", m_vendorID);
		jKv(j, "productID", m_productID);
		jKv(j, "SN", m_SN);
		jKv<float>(j, "vRangeDraw", m_vRangeDraw);

		return true;
	}

	bool _UVC::open(void)
	{
		IF_F(!check());
		IF__(m_bOpen, true);

		IF_F(!UVCopen());

		IF_F(!getProtocolVersion());
		IF_F(!getDeviceInfo());

		if (!setStreamType(m_streamType))
		{
			LOG_I("setStreamType " + i2str(m_streamType) + " failed");
			return false;
		}

		IF_F(!UVCsetVideoMode());
		IF_F(!UVCstreamStart());

		m_bOpen = true;
		return true;
	}

	void _UVC::close(void)
	{
		this->_VisionBase::close();

		UVCstreamClose();
		UVCclose();
	}

	bool _UVC::UVCopen(void)
	{
		uvc_error_t r;

		r = uvc_init(&m_pCtx, NULL);
		if (r < 0)
		{
			uvc_perror(r, "uvc_init");
			return false;
		}

		LOG_I("initialized");

		r = uvc_find_device(
			m_pCtx,
			&m_pDev,
			m_vendorID,
			m_productID,
			(m_SN.empty()) ? NULL : m_SN.c_str()); // filter devices: vendor_id, product_id, "serial_num"
		if (r < 0)
		{
			uvc_perror(r, "uvc_find_device");
			return false;
		}

		LOG_I("device found");

		r = uvc_open(m_pDev, &m_pHandleDev);
		if (r < 0)
		{
			uvc_perror(r, "uvc_open");
			return false;
		}

		LOG_I("device opened");

		uvc_print_diag(m_pHandleDev, stderr);

		return true;
	}

	void _UVC::UVCclose(void)
	{
		uvc_close(m_pHandleDev);
		uvc_unref_device(m_pDev);
		uvc_exit(m_pCtx);
	}

	bool _UVC::UVCsetVideoMode(void)
	{
		int height = 0;

		const uvc_format_desc_t *fmt_desc = uvc_get_format_descs(m_pHandleDev);
		const uvc_frame_desc_t *frame_desc = fmt_desc->frame_descs;

		m_vSizeRGB.x = frame_desc->wWidth;
		m_uvcFPS = 10000000 / frame_desc->dwDefaultFrameInterval;

		if (frame_desc->wWidth == 640)
		{
			m_vSizeRGB.y = 512;
			height = 1033;
			m_uvcLen = 640 * 512 * 2;
		}
		else if (frame_desc->wWidth == 384)
		{
			m_vSizeRGB.y = 288;
			height = 590;
			m_uvcLen = 384 * 288 * 2;
		}
		else if (frame_desc->wWidth == 256)
		{
			m_vSizeRGB.y = 192;
			height = 400;
			m_uvcLen = 256 * 192 * 2;
		}

		m_uvcSize = 4640 + 2 * m_uvcLen;
		m_uvcOffset = m_uvcLen + 4640;

		LOG_I("width=" + i2str(m_vSizeRGB.x) +
			  ", height=" + i2str(m_vSizeRGB.y) +
			  ", size=" + i2str(m_uvcSize) +
			  ", len=" + i2str(m_uvcLen) +
			  ", offset=" + i2str(m_uvcOffset) +
			  ", FPS=" + i2str(m_uvcFPS));

		uvc_error r = uvc_get_stream_ctrl_format_size(m_pHandleDev, &m_ctrl, UVC_FRAME_FORMAT_YUYV, // UVC_FRAME_FORMAT_UNKNOWN
													  m_vSizeRGB.x, height, m_uvcFPS);

		IF_F(r != UVC_SUCCESS);

		return true;
	}

	bool _UVC::UVCstreamStart(void)
	{
		uvc_error_t r;

		r = uvc_stream_open_ctrl(m_pHandleDev, &m_pHandleStream, &m_ctrl);
		if (r != UVC_SUCCESS)
		{
			LOG_I("open_ctrl error!");
			uvc_perror(r, "open_ctrl");
			return false;
		}

		//		r = uvc_stream_start(m_pHandleStream, sCbGetFrame, this, 0);
		r = uvc_stream_start(m_pHandleStream, NULL, this, 0);
		if (r != UVC_SUCCESS)
		{
			LOG_I("stream_start error!");
			uvc_perror(r, "stream_start");
			UVCstreamClose();
			return false;
		}

		return true;
	}

	void _UVC::UVCstreamClose(void)
	{
		IF_(!m_pHandleStream);

		uvc_stream_close(m_pHandleStream);
		m_pHandleStream = nullptr;
	}

	void _UVC::cbGetFrame(uvc_frame *pFrame)
	{
		NULL_(pFrame);
	}

	void _UVC::UVCstreamGetFrame(unsigned int tOut)
	{
		uvc_error_t r;

		m_pUVCframe = NULL;
		r = uvc_stream_get_frame(m_pHandleStream, &m_pUVCframe, tOut);
	}

	bool _UVC::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _UVC::update(void)
	{
		while (m_pT->bAlive())
		{
			if (!m_bOpen)
			{
				if (!open())
				{
					close(); // keep it, as it may requires several tries to get connected
					m_pT->sleepT(SEC_2_USEC);
					continue;
				}
			}

			m_pT->autoFPS();

			uvc_frame_t *pFrame = nullptr;
			uvc_error_t r;

			// Get pFrame (blocking call)
			r = uvc_stream_get_frame(m_pHandleStream, &pFrame, USEC_1SEC); // Timeout: 1 second
			if (r < 0 || !pFrame)
			{
				LOG_I("uvc_stream_get_frame: " + string(uvc_strerror(r)));
				continue;
			}

			if (pFrame->data_bytes != m_uvcSize)
			{
				LOG_I("Frame size wrong, " + i2str(pFrame->data_bytes) + " : " + i2str(m_uvcSize));
				continue;
			}

			// Mat mRGB;
			//  Mat mYUV = cv::Mat(m_vSizeRGB.y, m_vSizeRGB.x, CV_8UC2, ((unsigned char *)pFrame->data) + m_uvcOffset, Mat::AUTO_STEP);
			//  cv::cvtColor(mYUV, mRGB, COLOR_YUV2RGB_YUY2);
			//  int nHead = ((uint8_t *)pFrame->data)[4];

			Mat mRaw = cv::Mat(m_vSizeRGB.y, m_vSizeRGB.x, CV_16UC1, ((uint16_t *)(pFrame->data + 4640)), Mat::AUTO_STEP);

			const float tScale = (1.0 / 64.0);
			Mat mC;
			mRaw.convertTo(mC, CV_32FC1, tScale, -50.0);

			m_fRGB.copy(mC);
		}
	}

	int _UVC::USBctrlTransfer(unsigned char bmRequestType, unsigned char bRequest,
							  unsigned short wValue, unsigned short wIndex,
							  unsigned char *data, uint16_t wLength, unsigned int timeout)
	{
		return libusb_control_transfer(uvc_get_libusb_handle(m_pHandleDev), bmRequestType, bRequest, wValue, wIndex,
									   data, wLength, timeout);
	}

	int _UVC::USBgetLen(unsigned short wValue)
	{
		int r;
		unsigned char pData[2];
		int len = 2;

		r = USBctrlTransfer(REQ_TYPE_GET, REQ_GET_LEN, wValue, 0x0a00, pData, len, 0);
		IF__(r == len, *((unsigned short *)pData));

		return -1;
	}

	int _UVC::USBgetCur(unsigned short wValue, unsigned char *pData, unsigned short len)
	{
		int r, c = 0;

		r = USBctrlTransfer(REQ_TYPE_GET, REQ_GET_CUR, wValue, 0x0a00, pData, len, 0);
		IF__(r == len, len);

		return -1;
	}

	int _UVC::USBgetErrno(void)
	{
		int len;
		unsigned char pData[4];

		len = USBgetLen(XU_CS_ID_ERROR_CODE);
		if (len < 0)
			return -1;
		if (USBgetCur(XU_CS_ID_ERROR_CODE, pData, 1) < 0)
			return -1;

		printf("errno = %d\n", pData[0]);
		return (int)pData[0];
	}

	int _UVC::USBsetCur(unsigned short wValue, unsigned char *pData, unsigned short len)
	{
		int r, c = 0;

		r = USBctrlTransfer(REQ_TYPE_SET, REQ_SET_CUR, wValue, 0x0a00, pData, len, 0);
		IF__(r == len, len);

		return -1;
	}

	bool _UVC::USBsetCurFunc(unsigned short csID, unsigned char subID)
	{
		int len;

		len = USBgetLen(XU_CS_ID_COMMAND_SWITCH);
		IF_F(len != 2);

		unsigned char pFunc[2];
		pFunc[0] = (csID >> 8) & 0xff;
		pFunc[1] = subID;

		IF_F(USBsetCur(XU_CS_ID_COMMAND_SWITCH, pFunc, 2) < 0);

		return true;
	}

	int _UVC::USBsetCurData(unsigned short csID, unsigned char *pB)
	{
		int len;

		len = USBgetLen(csID);
		IF__(len < 0, -1);

		IF__(USBsetCur(csID, pB, len) < 0, -1);

		return len;
	}

	int _UVC::USBgetCurData(unsigned short csID, unsigned char *pB)
	{
		int len;

		len = USBgetLen(csID);
		IF__(len < 0, -1);

		IF__(USBgetCur(csID, pB, len) < 0, -1);

		return len;
	}

	bool _UVC::getProtocolVersion(void)
	{
		int len;
		unsigned char pData[4];

		len = USBgetLen(XU_CS_ID_PROTOCOL_VER);
		IF_F(len < 0);

		IF_F(USBgetCur(XU_CS_ID_PROTOCOL_VER, pData, len) < 0);
		printf("\n--- proto version: %s ---\n\n", pData);

		return true;
	}

	bool _UVC::setStreamType(unsigned char type)
	{
		unsigned char pData[1600] = {0};

		IF_F(!USBsetCurFunc(XU_CS_ID_THERMAL, THERMAL_STREAM_PARAM));

		IF_F(USBgetCurData(XU_CS_ID_THERMAL, pData) < 0);

		printf("0: chnlid = %d, stream_type = %d\n", pData[0], pData[1]);

		IF__(pData[1] == type, true);

		pData[1] = type;
		IF_F(USBsetCurData(XU_CS_ID_THERMAL, pData) < 0);

		USBgetCurData(XU_CS_ID_THERMAL, pData);
		printf("1: chnlid = %d, stream_type = %d\n", pData[0], pData[1]);

		IF__(pData[1] == type, true);
		return false;
	}

	bool _UVC::getDeviceInfo(void)
	{
		unsigned char pB[1600];

		printf("\n--- starting getDeviceInfo ----\n");
		IF_F(USBsetCurFunc(XU_CS_ID_SYSTEM, SYSTEM_DEVICE_INFO) < 0);
		IF_F(USBgetCurData(XU_CS_ID_SYSTEM, pB) < 0);

		uint8_t *ptr = pB;
		printf("\nfirmwareVersion: %s\n", ptr);
		ptr += 64;
		printf("encoderVersion: %s\n", ptr);
		ptr += 64;
		printf("hardwareVersion: %s\n", ptr);
		ptr += 64;
		printf("deviceName: %s\n", ptr);
		ptr += 64;
		printf("protocolVersion: %s\n", ptr);
		ptr += 4;
		printf("serialNumber: %s\n\n", ptr);

		char *pFW = (char *)pB;
		pFW = strstr(pFW, "BUILD");
		pFW += 6;
		unsigned int fwVer = (unsigned int)strtol(pFW, NULL, 10);
		printf("FW ver = %u\n", fwVer);

		return true;
	}

	void _UVC::draw(void *pFrame)
	{
		NULL_(pFrame);
		this->_ModuleBase::draw(pFrame);
		IF_(!check());
		IF_(m_fRGB.bEmpty());

		Frame *pF = (Frame *)pFrame;
		Mat mT = *m_fRGB.m();

		Mat mG;
		float range = m_vRangeDraw.len();
		float scale = 255.0 / range;
		mT.convertTo(mG,
					 CV_8UC1,
					 scale,
					 -m_vRangeDraw.x * scale);

		pF->copy(mG);
	}

}

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

		m_vendorID = 0;
		m_productID = 0;
		m_SN = "";
	}

	_UVC::~_UVC()
	{
		close();
	}

	int _UVC::init(void *pKiss)
	{
		CHECK_(_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vendorID", &m_vendorID);
		pK->v("productID", &m_productID);
		pK->v("SN", &m_SN);

		return OK_OK;
	}

	bool _UVC::open(void)
	{
		IF_F(check() != OK_OK);
		IF__(m_bOpen, true);

		IF_F(!UVCopen());

		getProtocolVersion();
		// if (get_device_info(30) != 0)
		// {
		// 	printf("\n--- get_device_info error ---\n");
		// 	return -1;
		// }

		if (!setStreamType(2))
		{
			printf("\n--- stream_type_config error ---\n");
			return false;
		}

		UVCsetVideoMode();
		UVCstreamStart();

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

	void _UVC::UVCsetVideoMode(void)
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

		printf("width=%d,height=%d,size=%d,len=%d,offset=%d\n", m_vSizeRGB.x, m_vSizeRGB.y, m_uvcSize, m_uvcLen, m_uvcOffset);

		 uvc_get_stream_ctrl_format_size(m_pHandleDev, &m_ctrl, UVC_FRAME_FORMAT_YUYV,
		 								m_vSizeRGB.x, height, m_uvcFPS);
		// uvc_get_stream_ctrl_format_size(m_pHandleDev, &m_ctrl, UVC_FRAME_FORMAT_UNKNOWN,
		// 								m_vSizeRGB.x, m_vSizeRGB.y, m_uvcFPS);
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
			uvc_stream_close(m_pHandleStream);
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

	int _UVC::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
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
					m_pT->sleepT(SEC_2_USEC);
					continue;
				}
			}

			m_pT->autoFPS();

			Mat mCam;

			// while (!m_UVC.read(mCam))
			// 	;
			// m_fRGB.copy(mCam);

			// if (m_bResetCam)
			// {
			// 	m_UVC.release();
			// 	m_bOpen = false;
			// }

			uvc_frame_t *frame = nullptr;
			uvc_error_t r;

			// Get frame (blocking call)
			r = uvc_stream_get_frame(m_pHandleStream, &frame, 1000000); // Timeout: 1 second
			if (r < 0 || !frame)
			{
				std::cerr << "Failed to get frame: " << uvc_strerror(r) << std::endl;
				continue;
			}

			if (frame->data_bytes != m_uvcSize)
			{
				std::cerr << "Frame size wrong" << std::endl;
				continue;
			}

			Mat mRGB;
			// Mat mYUV = cv::Mat(m_vSizeRGB.y, m_vSizeRGB.x, CV_8UC2, ((unsigned char *)frame->data) + m_uvcOffset, Mat::AUTO_STEP);
			// cv::cvtColor(mYUV, mRGB, COLOR_YUV2RGB_YUY2);

			int nHead = ((uint8_t*)frame->data)[4];
			Mat mThermal = cv::Mat(m_vSizeRGB.y, m_vSizeRGB.x, CV_16UC1, ((uint16_t *)(frame->data + 4640)), Mat::AUTO_STEP);
			// uint16_t* pD = (uint16_t*)(frame->data + 4640);
			// uint16_t t = pD[10];
			// printf("Data size: %i, t_sample: %f\n", frame->data_bytes, ((float)t)/64.0 - 50);

			Mat mA;
			mThermal.convertTo(mA, CV_32FC1, 1.0/64, -50.0);
			printf("t_sample: %f\n", mA.at<float>(320,240));

			// Mat mRange;
			// cv::inRange(mA,
			// 		cv::Scalar(10),
			// 		cv::Scalar(80), mRange);
			// Mat mC;
			// mRange.convertTo(mC, CV_8UC1);

			Mat mG;
			float range = 40;
			float scale = 255.0 / range;
			mA.convertTo(mG,
						 CV_8UC1,
						 scale,
						 0);//-range * scale);

			cv::cvtColor(mG, mRGB, COLOR_GRAY2BGR);

			m_fRGB.copy(mRGB);
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

	void _UVC::dump_devinfo(unsigned char *ptr)
	{
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
	}

	unsigned int _UVC::get_fw_version(uint8_t *d)
	{
		char *ptr = (char *)d;

		ptr = strstr(ptr, "BUILD");
		ptr += 6;
		return (unsigned int)strtol(ptr, NULL, 10);
	}

	bool _UVC::get_device_info(void)
	{
		unsigned char pB[1600];
		unsigned int fw_ver;

		printf("\n--- starting get_device_info ----\n");
		IF_F(USBsetCurFunc(XU_CS_ID_SYSTEM, SYSTEM_DEVICE_INFO) < 0);
		IF_F(USBgetCurData(XU_CS_ID_SYSTEM, pB) < 0);
		dump_devinfo(pB);
		fw_ver = get_fw_version(pB);
		printf("fw_ver = %u\n", fw_ver);

		return true;
	}

}

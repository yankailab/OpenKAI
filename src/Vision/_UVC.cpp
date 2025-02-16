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

		/* set stream type: temperature+yuv */
		if (stream_type_config(STREAM_TYPE_YUV_TEMP, 20) != 0)
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
		//		return (int)err;
	}

	int _UVC::UVCctrlTransfer(unsigned char bmRequestType, unsigned char bRequest,
						 unsigned short wValue, unsigned short wIndex,
						 unsigned char *data, uint16_t wLength, unsigned int timeout)
	{
		libusb_device_handle* pHandleUSB = uvc_get_libusb_handle(m_pHandleDev);

		return libusb_control_transfer(pHandleUSB, bmRequestType, bRequest, wValue, wIndex,
									   data, wLength, timeout);
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
			Mat mYUV = cv::Mat(m_vSizeRGB.y, m_vSizeRGB.x, CV_8UC2, ((unsigned char *)frame->data) + m_uvcOffset, Mat::AUTO_STEP);
			cv::cvtColor(mYUV, mRGB, COLOR_YUV2RGB_YUY2);

			m_fRGB.copy(mRGB);
		}
	}


	int _UVC::get_len(unsigned short wValue, int retries)
	{
		int err, c = 0;
		unsigned char data[2];

		while (c < retries)
		{
			err = UVCctrlTransfer(REQ_TYPE_GET, REQ_GET_LEN, wValue, 0x0a00, data, 2, 0);
			if (err == 2)
				return *((unsigned short *)data);

			printf("--- get_len error: %d ---\n", err);
			++c;
			usleep(500000);
		}

		return -1;
	}

	int _UVC::get_cur(unsigned short wValue, unsigned char *data, unsigned short len, int retries)
	{
		int err, c = 0;

		while (c < retries)
		{
			err = UVCctrlTransfer(REQ_TYPE_GET, REQ_GET_CUR, wValue, 0x0a00, data, len, 0);
			if (err == len)
				return len;

			printf("--- get_cur error: %d ---\n", err);
			++c;
			usleep(500000);
		}

		return -1;
	}

	int _UVC::get_errno(void)
	{
		int len;
		unsigned char data[4];

		len = get_len(XU_CS_ID_ERROR_CODE, 10);
		if (len < 0)
			return -1;
		if (get_cur(XU_CS_ID_ERROR_CODE, data, 1, 10) < 0)
			return -1;

		printf("errno = %d\n", data[0]);
		return (int)data[0];
	}

	int _UVC::set_cur(unsigned short wValue, unsigned char *data, unsigned short len, int retries)
	{
		int err, c = 0;

		while (c < retries)
		{
			err = UVCctrlTransfer(REQ_TYPE_SET, REQ_SET_CUR, wValue, 0x0a00, data, len, 0);
			if (err == len)
				return len;
			printf("--- set_cur error: %d ---\n", err);
			++c;
			usleep(500000);
		}

		return -1;
	}

	int _UVC::set_curr_func(unsigned short cs_id, unsigned char sub_id, int retries)
	{
		int len;
		unsigned char func[2];

		while (1)
		{
			len = get_len(XU_CS_ID_COMMAND_SWITCH, retries);
			if (len == 2)
				break;
			printf("--- get switch cmd len error: %d. ---\n", len);
			usleep(500000);
		}

		func[0] = (cs_id >> 8) & 0xff;
		func[1] = sub_id;
		if (set_cur(XU_CS_ID_COMMAND_SWITCH, func, 2, retries) < 0)
		{
			printf("--- set switch cmd error. ---\n");
			return -1;
		}

		return 0;
	}

	int _UVC::set_curr_data(unsigned short cs_id, unsigned char *buf, int retries)
	{
		int len;

		len = get_len(cs_id, retries);
		if (len < 0)
		{
			printf("--- get_len error. ---\n");
			return -1;
		}
		printf("\n--- set_curr_data: len = %d ---\n\n", len);
		if (set_cur(cs_id, buf, len, retries) < 0)
		{
			printf("--- set_curr error. ---\n");
			return -1;
		}

		return len;
	}

	int _UVC::get_curr_data(unsigned short cs_id, unsigned char *buf, int retries)
	{
		int len;

		len = get_len(cs_id, retries);
		if (len < 0)
		{
			printf("--- get_len error. ---\n");
			return -1;
		}

		printf("\n--- get_curr_data: len = %d ---\n\n", len);

		if (get_cur(cs_id, buf, len, retries) < 0)
		{
			printf("-- get_curr error. ---\n");
			return -1;
		}

		return len;
	}

	int _UVC::get_protocol_version(int retries)
	{
		int len;
		unsigned char data[4];

		len = get_len(XU_CS_ID_PROTOCOL_VER, retries);
		if (len < 0)
		{
			printf("--- get version len error. ---\n");
			return -1;
		}
		printf("--- proto version len: %d ---\n", len);
		if (get_cur(XU_CS_ID_PROTOCOL_VER, data, len, retries) < 0)
		{
			printf("--- get version error. ---\n");
			return -1;
		}
		printf("\n--- proto version: %s ---\n\n", data);

		return 0;
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

	int _UVC::get_device_info(int retries)
	{
		unsigned char rbuf[1600];
		unsigned int fw_ver;

		printf("\n--- starting get_device_info ----\n");
		if (set_curr_func(XU_CS_ID_SYSTEM, SYSTEM_DEVICE_INFO, retries) < 0)
			return -1;
		if (get_curr_data(XU_CS_ID_SYSTEM, rbuf, retries) < 0)
			return -1;
		dump_devinfo(rbuf);
		fw_ver = get_fw_version(rbuf);
		printf("fw_ver = %u\n", fw_ver);

		//	if (set_curr_func(u, XU_CS_ID_THERMAL, THERMAL_ALG_VERSION) < 0) return -1;
		//	if (get_curr_data(u, XU_CS_ID_THERMAL, rbuf) < 0) return -1;
		return 0;
	}

	int _UVC::wait_cmd_done(int after, int repeat)
	{
		int r = 0;

		usleep(after * 1000);
		while ((r = get_errno()) == 1)
			usleep(repeat * 1000);

		printf("finally errno = %d\n", r);
		return r;
	}

	int _UVC::stream_type_config(unsigned char type, int retries)
	{
		unsigned char data[1600] = {0};

		printf("\n--- starting stream_type_config ----\n");
		if (set_curr_func(XU_CS_ID_THERMAL, THERMAL_STREAM_PARAM, retries) != 0)
			return -1;
		wait_cmd_done(15, 15);

		if (get_curr_data(XU_CS_ID_THERMAL, data, retries) < 0)
			return -1;
		printf("0: chnlid = %d, stream_type = %d\n", data[0], data[1]);

		if (data[1] != type)
		{
			data[1] = type;
			if (set_curr_data(XU_CS_ID_THERMAL, data, retries) < 0)
				return -1;

			wait_cmd_done(100, 100);

			get_curr_data(XU_CS_ID_THERMAL, data, retries);
			printf("1: chnlid = %d, stream_type = %d\n", data[0], data[1]);
		}
		else
		{
			printf("---- stream_type meets my demands and not config again! ------\n");
		}

		return 0;
	}



}

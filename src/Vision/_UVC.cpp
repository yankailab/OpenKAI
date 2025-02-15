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

		r = uvc_stream_start(m_pHandleStream, sCbGetFrame, this, 0);
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
		}
	}
}

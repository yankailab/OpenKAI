/*
 * _Orbbec.cpp
 *
 *  Created on: Feb 13, 2023
 *      Author: yankai
 */

#include "_Orbbec.h"

namespace kai
{

	_Orbbec::_Orbbec()
	{

	}

	_Orbbec::~_Orbbec()
	{
	}

	int _Orbbec::init(void *pKiss)
	{
		CHECK_(_RGBDbase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

        Kiss *pKt = pK->child("threadPP");
        if (pKt->empty())
        {
            LOG_E("threadPP not found");
            return OK_ERR_NOT_FOUND;
        }

        m_pTPP = new _Thread();
        CHECK_d_l_(m_pTPP->init(pKt), DEL(m_pTPP), "threadPP init failed");

		return OK_OK;
	}

	int _Orbbec::link(void)
	{
		CHECK_(this->_RGBDbase::link());
		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		return OK_OK;
	}

	bool _Orbbec::open(void)
	{
		IF__(m_bOpen, true);

		m_spObConfig = std::make_shared<ob::Config>();
		m_spObConfig->enableVideoStream(OB_STREAM_COLOR);
		m_obPipe.start(m_spObConfig);

		m_bOpen = true;
		return true;
	}

	void _Orbbec::close(void)
	{
		IF_(!m_bOpen);

		m_obPipe.stop();
	}

	int _Orbbec::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		NULL__(m_pTPP, OK_ERR_NULLPTR);
		CHECK_(m_pT->start(getUpdate, this));
		return m_pTPP->start(getTPP, this);
	}

	int _Orbbec::check(void)
	{
		NULL__(m_pTPP, OK_ERR_NULLPTR);

		return this->_RGBDbase::check();
	}

	void _Orbbec::update(void)
	{
		while (m_pT->bAlive())
		{
			if (!m_bOpen)
			{
				if (!open())
				{
					LOG_E("Cannot open Orbbec");
					m_pT->sleepT(SEC_2_USEC);
					continue;
				}
			}

			m_pT->autoFPS();

			if (updateOrbbec())
			{
				m_pTPP->run();
				//				updatePC();
			}
			else
			{
				m_pT->sleepT(SEC_2_USEC);
				m_bOpen = false;
			}

		}
	}

	bool _Orbbec::updateOrbbec(void)
	{
		IF__(check() != OK_OK, true);

        // Wait for up to 100ms for a frameset in blocking mode.
        auto obFrameSet = m_obPipe.waitForFrameset();
        NULL_F(obFrameSet);

        // get color frame from frameset.
        auto obColorFrame = obFrameSet->getFrame(OB_FRAME_COLOR);

        // Render colorFrame.
//        win.pushFramesToView(obColorFrame);
		obColorFrame->getData();


//		updateRGBD(frameReady);
//		updatePointCloud(frameReady);

		return true;
	}

	// bool _Orbbec::updateRGBD(const VzFrameReady &vfr)
	// {
	// 	VzReturnStatus status;

	// 	if (m_bRGB && vfr.color == 1)
	// 	{
	// 		status = VZ_GetFrame(m_deviceHandle, VzColorFrame, &m_vzfRGB);
	// 		if (m_vzfRGB.pFrameData)
	// 			memcpy(m_psmRGB->p(), m_vzfRGB.pFrameData, m_vzfRGB.dataLen);
	// 	}

	// 	if (m_btRGB && vfr.transformedColor == 1)
	// 	{
	// 		status = VZ_GetFrame(m_deviceHandle, VzTransformColorImgToDepthSensorFrame, &m_vzfTransformedRGB);
	// 		if (m_vzfTransformedRGB.pFrameData)
	// 			memcpy(m_psmTransformedRGB->p(),
	// 				   m_vzfTransformedRGB.pFrameData,
	// 				   m_vzfTransformedRGB.dataLen);
	// 	}

	// 	if (m_bDepth && vfr.depth == 1)
	// 	{
	// 		status = VZ_GetFrame(m_deviceHandle, VzDepthFrame, &m_vzfDepth);
	// 		if (m_vzfDepth.pFrameData)
	// 			memcpy(m_psmDepth->p(), m_vzfDepth.pFrameData, m_vzfDepth.dataLen);
	// 	}

	// 	if (m_btDepth && vfr.transformedDepth == 1)
	// 	{
	// 		status = VZ_GetFrame(m_deviceHandle, VzTransformDepthImgToColorSensorFrame, &m_vzfTransformedDepth);
	// 		if (m_vzfTransformedDepth.pFrameData)
	// 			memcpy(m_psmTransformedDepth->p(),
	// 				   m_vzfTransformedDepth.pFrameData,
	// 				   m_vzfTransformedDepth.dataLen);
	// 	}

	// 	if (m_bIR && vfr.ir == 1)
	// 	{
	// 		status = VZ_GetFrame(m_deviceHandle, VzIRFrame, &m_vzfIR);
	// 		if (m_vzfIR.pFrameData)
	// 			memcpy(m_psmIR->p(), m_vzfIR.pFrameData, m_vzfIR.dataLen);
	// 	}

	// 	return true;
	// }

	// bool _Orbbec::updatePointCloud(const VzFrameReady &vfr)
	// {


	// 	return true;
	// }

	void _Orbbec::updateTPP(void)
	{
		while (m_pTPP->bAlive())
		{
			m_pTPP->sleepT(0);

			//            m_fDepth.copy(mDs + m_dOfs);
			// if (m_bDepthShow)
			// {
			//     IF_(m_fDepth.bEmpty());

			//     dispImg = cv::Mat(height, width, CV_16UC1, pData);

			//     dispImg.convertTo(dispImg, CV_8U, 255.0 / slope);
			//     applyColorMap(dispImg, dispImg, cv::COLORMAP_RAINBOW);

			//     Mat mDColor(Size(m_vDsize.x, m_vDsize.y), CV_8UC3, (void *)dColor.get_data(),
			//                 Mat::AUTO_STEP);
			//     m_fDepthShow.copy(mDColor);
			// }
		}
	}

}

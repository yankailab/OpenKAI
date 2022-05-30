/*
 * _VisionBase.cpp
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#include "_VisionBase.h"

namespace kai
{

	_VisionBase::_VisionBase()
	{
		m_bOpen = false;
		m_type = vision_unknown;
		m_vSize.set(1280, 720);
		m_bbDraw.x = -1.0;
	}

	_VisionBase::~_VisionBase()
	{
	}

	bool _VisionBase::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vSize", &m_vSize);
		pK->v("bbDraw", &m_bbDraw);

		m_bOpen = false;

		return true;
	}

	Frame *_VisionBase::BGR(void)
	{
		return &m_fBGR;
	}

	vInt2 _VisionBase::getSize(void)
	{
		return m_vSize;
	}

	VISION_TYPE _VisionBase::getType(void)
	{
		return m_type;
	}

	bool _VisionBase::open(void)
	{
		return true;
	}

	bool _VisionBase::isOpened(void)
	{
		return m_bOpen;
	}

	void _VisionBase::close(void)
	{
		IF_(check() < 0);

		m_pT->goSleep();
		while (!m_pT->bSleeping())
			;

		m_bOpen = false;
	}

	void _VisionBase::draw(void* pFrame)
	{
#ifdef WITH_UI
#ifdef USE_OPENCV
		NULL_(pFrame);
		this->_ModuleBase::draw(pFrame);
		IF_(check() < 0);

		_WindowCV *pWin = (_WindowCV *)pFrame;
		Frame *pF = pWin->getNextFrame();
		NULL_(pF);

		IF_(m_fBGR.bEmpty());

		if (m_bbDraw.x < 0.0)
		{
			pF->copy(m_fBGR);
		}
		else
		{
			Mat *pM = pF->m();
			IF_(pM->empty());
			Rect r = bb2Rect(bbScale(m_bbDraw, pM->cols, pM->rows));
			Mat m;
			cv::resize(*m_fBGR.m(), m, Size(r.width, r.height));

			m.copyTo((*pM)(r));
		}
#endif
#endif
	}

}

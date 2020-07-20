/*
 * _Crop.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_Crop.h"

#ifdef USE_OPENCV

namespace kai
{

_Crop::_Crop()
{
	m_type = vision_crop;
	m_pV = NULL;

	m_roi.init();
	m_roi.z = 1.0;
	m_roi.w = 1.0;
}

_Crop::~_Crop()
{
	close();
}

bool _Crop::init(void* pKiss)
{
	IF_F(!_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("roi",&m_roi);

	string iName;
	iName = "";
	pK->v("_VisionBase", &iName);
	m_pV = (_VisionBase*) (pK->getInst(iName));
	IF_Fl(!m_pV, iName + ": not found");

	return true;
}

bool _Crop::open(void)
{
	NULL_F(m_pV);
	m_bOpen = m_pV->isOpened();

	return m_bOpen;
}

void _Crop::close(void)
{
	if(m_threadMode==T_THREAD)
	{
		goSleep();
		while(!bSleeping());
	}

	this->_VisionBase::close();
}

bool _Crop::start(void)
{
	IF_F(!this->_ThreadBase::start());

	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _Crop::update(void)
{
	while (m_bThreadON)
	{
		if (!m_bOpen)
			open();

		this->autoFPSfrom();

		if(m_bOpen)
		{
			if(m_fIn.tStamp() < m_pV->BGR()->tStamp())
				filter();
		}

		this->autoFPSto();
	}
}

void _Crop::filter(void)
{
	IF_(m_pV->BGR()->bEmpty());

	Mat mIn = *m_pV->BGR()->m();
	Rect r(m_roi.x * mIn.cols,
		   m_roi.y * mIn.rows,
		   m_roi.width() * mIn.cols,
		   m_roi.height() * mIn.rows);

	m_w = r.width;
	m_h = r.height;

	m_fBGR.copy(mIn(r));
}

}
#endif

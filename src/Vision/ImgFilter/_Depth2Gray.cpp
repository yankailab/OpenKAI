/*
 * _Depth2Gray.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_Depth2Gray.h"

namespace kai
{

	_Depth2Gray::_Depth2Gray()
	{
		m_type = vision_depth2Gray;
		m_pV = NULL;
	}

	_Depth2Gray::~_Depth2Gray()
	{
		close();
	}

	bool _Depth2Gray::init(void *pKiss)
	{
		IF_F(!_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		string n;
		n = "";
		pK->v("_DepthVisionBase", &n);
		m_pV = (_DepthVisionBase *)(pK->getInst(n));
		IF_Fl(!m_pV, n + ": not found");

		return true;
	}

	bool _Depth2Gray::open(void)
	{
		NULL_F(m_pV);
		m_bOpen = m_pV->isOpened();

		return m_bOpen;
	}

	void _Depth2Gray::close(void)
	{
		this->_VisionBase::close();
	}

	bool _Depth2Gray::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _Depth2Gray::update(void)
	{
		while (m_pT->bRun())
		{
			if (!m_bOpen)
				open();

			m_pT->autoFPSfrom();

			if (m_bOpen)
			{
				filter();
			}

			m_pT->autoFPSto();
		}
	}

	void _Depth2Gray::filter(void)
	{
		IF_(m_pV->Depth()->bEmpty());

		Mat mD = *m_pV->Depth()->m();
		Mat mGray;
		float scale = 255.0 / m_pV->m_vRange.len();
		mD.convertTo(mGray,
					 CV_8UC1,
					 scale,
					 -m_pV->m_vRange.x * scale);
		m_fBGR.copy(mGray);
	}
}

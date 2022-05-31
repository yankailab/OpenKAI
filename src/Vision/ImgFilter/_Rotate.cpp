/*
 * _Rotate.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_Rotate.h"

namespace kai
{

	_Rotate::_Rotate()
	{
		m_type = vision_rotate;
		m_pV = NULL;
		m_code = 0;
	}

	_Rotate::~_Rotate()
	{
		close();
	}

	bool _Rotate::init(void *pKiss)
	{
		IF_F(!_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("code", &m_code);

		string n;
		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->getInst(n));
		IF_Fl(!m_pV, n + ": not found");

		return true;
	}

	bool _Rotate::open(void)
	{
		NULL_F(m_pV);
		m_bOpen = m_pV->isOpened();

		return m_bOpen;
	}

	void _Rotate::close(void)
	{
		this->_VisionBase::close();
	}

	bool _Rotate::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _Rotate::update(void)
	{
		while (m_pT->bRun())
		{
			if (!m_bOpen)
				open();

			m_pT->autoFPSfrom();

			if (m_bOpen)
			{
				if (m_fBGR.tStamp() < m_pV->BGR()->tStamp())
					filter();
			}

			m_pT->autoFPSto();
		}
	}

	void _Rotate::filter(void)
	{
		IF_(m_pV->BGR()->bEmpty());

		Mat m;
		cv::rotate(*m_pV->BGR()->m(), m, m_code);
		m_fBGR.copy(m);
	}

}

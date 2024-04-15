/*
 * _DepthShow.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_DepthShow.h"

namespace kai
{

	_DepthShow::_DepthShow()
	{
		m_type = vision_RSdepth;
		m_pV = NULL;
	}

	_DepthShow::~_DepthShow()
	{
		close();
	}

	bool _DepthShow::init(void *pKiss)
	{
		IF_F(!_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		string n;
		n = "";
		pK->v("_RGBDbase", &n);
		m_pV = (_RGBDbase *)(pK->getInst(n));
		IF_Fl(!m_pV, "_RGBDbase not found: " + n);

		return true;
	}

	bool _DepthShow::open(void)
	{
		NULL_F(m_pV);
		m_bOpen = m_pV->isOpened();

		return m_bOpen;
	}

	void _DepthShow::close(void)
	{
		this->_VisionBase::close();
	}

	bool _DepthShow::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _DepthShow::update(void)
	{
		while (m_pT->bAlive())
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

	void _DepthShow::filter(void)
	{
//		IF_(m_pV->DepthShow()->bEmpty());

//		Mat mD = *m_pV->DepthShow()->m();
//		m_fRGB.copy(mD);
	}

}

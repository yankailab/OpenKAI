/*
 * _Crop.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_Crop.h"

namespace kai
{

	_Crop::_Crop()
	{
		m_type = vision_crop;
		m_pV = NULL;

		m_vRoi.clear();
	}

	_Crop::~_Crop()
	{
		close();
	}

	bool _Crop::init(void *pKiss)
	{
		IF_F(!_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vRoi", &m_vRoi);

		return true;
	}

	bool _Crop::link(void)
	{
		IF_F(!this->_VisionBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		string n;
		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->findModule(n));
		IF_Fl(!m_pV, n + ": not found");

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
		this->_VisionBase::close();
	}

	bool _Crop::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _Crop::update(void)
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

	void _Crop::filter(void)
	{
		IF_(m_pV->getFrameRGB()->bEmpty());

		Mat mIn = *m_pV->getFrameRGB()->m();
		Rect r;
		r.x = constrain(m_vRoi.x, 0, mIn.cols);
		r.y = constrain(m_vRoi.y, 0, mIn.rows);
		r.width = m_vRoi.z - r.x;
		r.height = m_vRoi.w - r.y;

		m_vSizeRGB.x = r.width;
		m_vSizeRGB.y = r.height;

		m_fRGB.copy(mIn(r));
	}

}

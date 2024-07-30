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
		m_pV = nullptr;

		m_vRoi.clear();
	}

	_Crop::~_Crop()
	{
		close();
	}

	int _Crop::init(void *pKiss)
	{
		CHECK_(_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vRoi", &m_vRoi);

		return OK_OK;
	}

	int _Crop::link(void)
	{
		CHECK_(this->_VisionBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		string n;
		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->findModule(n));
		NULL__(m_pV, OK_ERR_NOT_FOUND);

		return OK_OK;
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

	int _Crop::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
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

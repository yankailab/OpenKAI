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
		m_pV = nullptr;
	}

	_Depth2Gray::~_Depth2Gray()
	{
		close();
	}

	int _Depth2Gray::init(void *pKiss)
	{
		CHECK_(_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		string n;
		n = "";
		pK->v("_RGBDbase", &n);
		m_pV = (_RGBDbase *)(pK->findModule(n));
		NULL__(m_pV, OK_ERR_NOT_FOUND);

		return OK_OK;
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

	int _Depth2Gray::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	void _Depth2Gray::update(void)
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

	void _Depth2Gray::filter(void)
	{
		IF_(m_pV->getFrameD()->bEmpty());

		Mat mD = *m_pV->getFrameD()->m();
		Mat mGray;
		float scale = 255.0 / m_pV->getRangeD().len();
		mD.convertTo(mGray,
					 CV_8UC1,
					 scale,
					 -m_pV->getRangeD().x * scale);
		m_fRGB.copy(mGray);
	}
}

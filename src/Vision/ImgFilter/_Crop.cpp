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

		m_roi.init();
		m_roi.z = 1.0;
		m_roi.w = 1.0;
	}

	_Crop::~_Crop()
	{
		close();
	}

	bool _Crop::init(void *pKiss)
	{
		IF_F(!_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("roi", &m_roi);

		string n;
		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->getInst(n));
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
		while (m_pT->bRun())
		{
			if (!m_bOpen)
				open();

			m_pT->autoFPSfrom();

			if (m_bOpen)
			{
				if (m_fIn.tStamp() < m_pV->BGR()->tStamp())
					filter();
			}

			m_pT->autoFPSto();
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

		m_vSize.x = r.width;
		m_vSize.y = r.height;

		m_fBGR.copy(mIn(r));
	}

}

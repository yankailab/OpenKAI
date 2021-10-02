/*
 * _Remap.cpp
 *
 *  Created on: May 7, 2021
 *      Author: yankai
 */

#include "_Remap.h"

#ifdef USE_OPENCV

namespace kai
{

	_Remap::_Remap()
	{
		m_type = vision_remap;
		m_pV = NULL;
		m_bReady = false;
	}

	_Remap::~_Remap()
	{
		close();
	}

	bool _Remap::init(void *pKiss)
	{
		IF_F(!_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		string n;
		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->getInst(n));
		IF_Fl(!m_pV, n + ": not found");

		return true;
	}

	bool _Remap::open(void)
	{
		NULL_F(m_pV);
		m_bOpen = m_pV->isOpened();

		return m_bOpen;
	}

	void _Remap::close(void)
	{
		this->_VisionBase::close();
	}

	bool _Remap::bReady(void)
	{
		return m_bReady;
	}

	bool _Remap::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _Remap::update(void)
	{
		while (m_pT->bRun())
		{
			if (!m_bOpen)
			{
				open();
				if (!m_bOpen)
					continue;
			}

			m_pT->autoFPSfrom();

			filter();

			m_pT->autoFPSto();
		}
	}

	void _Remap::filter(void)
	{
		Frame *pF = m_pV->BGR();
		IF_(pF->bEmpty());
		IF_(m_fBGR.tStamp() >= pF->tStamp())

		if(!m_bReady || pF->size() != cv::Size(m_vSize.x, m_vSize.y))
		{
			cv::Size s = pF->size();
			m_vSize.x = s.width;
			m_vSize.y = s.height;
			m_bReady = scaleCamMatrices();
		}

		if(m_bReady)
			m_fBGR.copy(pF->remap(m_m1, m_m2));
		else
			m_fBGR.copy(*pF);
	}
}
#endif

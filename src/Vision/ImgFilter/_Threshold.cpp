/*
 * _Threshold.cpp
 *
 *  Created on: March 12, 2019
 *      Author: yankai
 */

#include "_Threshold.h"

namespace kai
{

	_Threshold::_Threshold()
	{
		m_type = vision_threshold;
		m_pV = NULL;
	}

	_Threshold::~_Threshold()
	{
		close();
	}

	bool _Threshold::init(void *pKiss)
	{
		IF_F(!_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		int i = 0;
		while (1)
		{
			Kiss *pM = pK->child(i++);
			if (pM->empty())
				break;

			IMG_THRESHOLD t;
			t.init();
			pM->v("type", &t.m_type);
			pM->v("vMax", &t.m_vMax);
			pM->v("bAutoThr", &t.m_bAutoThr);
			pM->v("thr", &t.m_thr);
			pM->v("method", &t.m_method);
			pM->v("thrType", &t.m_thrType);
			pM->v("blockSize", &t.m_blockSize);
			pM->v("C", &t.m_C);

			m_vFilter.push_back(t);
		}

		string n;
		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->getInst(n));
		IF_Fl(!m_pV, n + ": not found");

		return true;
	}

	bool _Threshold::open(void)
	{
		NULL_F(m_pV);
		m_bOpen = m_pV->isOpened();

		return m_bOpen;
	}

	void _Threshold::close(void)
	{
		this->_VisionBase::close();
	}

	bool _Threshold::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _Threshold::update(void)
	{
		while (m_pT->bAlive())
		{
			if (!m_bOpen)
				open();

			m_pT->autoFPSfrom();

			if (m_bOpen)
			{
				if (m_fIn.tStamp() < m_pV->getFrameRGB()->tStamp())
				{
					filter();
				}
			}

			m_pT->autoFPSto();
		}
	}

	void _Threshold::filter(void)
	{
		IF_(m_pV->getFrameRGB()->bEmpty());

		if (m_pV->getFrameRGB()->m_mat.type() != CV_8UC1)
			m_fIn.copy(m_pV->getFrameRGB()->cvtColor(COLOR_RGB2GRAY));
		else
			m_fIn.copy(*m_pV->getFrameRGB());

		Mat m1 = *m_fIn.m();
		Mat m2;
		Mat *pM1 = &m1;
		Mat *pM2 = &m2;
		Mat *pT;

		for (int i = 0; i < m_vFilter.size(); i++)
		{
			IMG_THRESHOLD *pI = &m_vFilter[i];

			if (pI->m_type == img_thr_adaptive)
			{
				cv::adaptiveThreshold(*pM1, *pM2,
									  pI->m_vMax,
									  pI->m_method,
									  pI->m_thrType,
									  pI->m_blockSize,
									  pI->m_C);
			}
			else if (pI->m_type == img_thr)
			{
				if (pI->m_bAutoThr)
				{
					cv::threshold(*pM1, *pM2,
								  0,
								  255,
								  pI->m_thrType | THRESH_OTSU);
				}
				else
				{
					cv::threshold(*pM1, *pM2,
								  pI->m_thr,
								  pI->m_vMax,
								  pI->m_thrType);
				}
			}

			SWAP(pM1, pM2, pT);
		}

		m_fRGB.copy(*pM1);
	}

}

/*
 * _Morphology.cpp
 *
 *  Created on: March 11, 2019
 *      Author: yankai
 */

#include "_Morphology.h"

namespace kai
{

	_Morphology::_Morphology()
	{
		m_type = vision_morphology;
		m_pV = NULL;
	}

	_Morphology::~_Morphology()
	{
		close();
	}

	bool _Morphology::init(void *pKiss)
	{
		IF_F(!_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		int i = 0;
		while (1)
		{
			Kiss *pM = pK->child(i++);
			if (pM->empty())
				break;

			IMG_MORPH m;
			m.init();
			pM->v("morphOp", &m.m_morphOp);
			pM->v("nItr", &m.m_nItr);
			pM->v("kShape", &m.m_kShape);
			pM->v("kW", &m.m_kW);
			pM->v("kH", &m.m_kH);
			pM->v("aX", &m.m_aX);
			pM->v("aY", &m.m_aY);
			m.updateKernel();

			m_vFilter.push_back(m);
		}

		string n;
		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->findModule(n));
		IF_Fl(!m_pV, n + ": not found");

		return true;
	}

	bool _Morphology::open(void)
	{
		NULL_F(m_pV);
		m_bOpen = m_pV->isOpened();

		return m_bOpen;
	}

	void _Morphology::close(void)
	{
		this->_VisionBase::close();
	}

	bool _Morphology::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _Morphology::update(void)
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

	void _Morphology::filter(void)
	{
		IF_(m_pV->getFrameRGB()->bEmpty());

		m_fIn.copy(*m_pV->getFrameRGB());

		Mat m1 = *m_fIn.m();
		Mat m2;
		Mat *pM1 = &m1;
		Mat *pM2 = &m2;
		Mat *pT;

		for (int i = 0; i < m_vFilter.size(); i++)
		{
			IMG_MORPH *pM = &m_vFilter[i];

			cv::morphologyEx(*pM1, *pM2,
							 pM->m_morphOp,
							 pM->m_kernel,
							 cv::Point(pM->m_aX, pM->m_aY),
							 pM->m_nItr);

			SWAP(pM1, pM2, pT);
		}

		m_fRGB.copy(*pM1);
	}

}

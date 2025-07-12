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
		m_pV = nullptr;
	}

	_Morphology::~_Morphology()
	{
	}

	int _Morphology::init(void *pKiss)
	{
		CHECK_(_VisionBase::init(pKiss));
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

		return OK_OK;
	}

	int _Morphology::link(void)
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

	int _Morphology::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	void _Morphology::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _Morphology::filter(void)
	{
		NULL_(m_pV);
		Frame *pF = m_pV->getFrameRGB();
		IF_(pF->bEmpty());
		IF_(m_fIn.tStamp() >= pF->tStamp());

		m_fIn.copy(*pF);

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

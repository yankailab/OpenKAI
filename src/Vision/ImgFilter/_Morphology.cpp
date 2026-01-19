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

	bool _Morphology::init(const json &j)
	{
		IF_F(!_VisionBase::init(j));

		const json &jF = j.at("filters");
		IF__(!jF.is_object(), true);

		for (auto it = jF.begin(); it != jF.end(); it++)
		{
			const json &Ji = it.value();
			IF_CONT(!Ji.is_object());

			IMG_MORPH m;
			m.init();
			jVar(Ji, "morphOp", m.m_morphOp);
			jVar(Ji, "nItr", m.m_nItr);
			jVar(Ji, "kShape", m.m_kShape);
			jVar(Ji, "kW", m.m_kW);
			jVar(Ji, "kH", m.m_kH);
			jVar(Ji, "aX", m.m_aX);
			jVar(Ji, "aY", m.m_aY);
			m.updateKernel();

			m_vFilter.push_back(m);
		}

		return true;
	}

	bool _Morphology::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_VisionBase::link(j, pM));

		string n = "";
		jVar(j, "_VisionBase", n);
		m_pV = (_VisionBase *)(pM->findModule(n));
		NULL_F(m_pV);

		return true;
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
			m_pT->autoFPS();

			filter();
		}
	}

	void _Morphology::filter(void)
	{
		NULL_(m_pV);
		Frame *pF = m_pV->getFrameRGB();
		NULL_(pF);
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

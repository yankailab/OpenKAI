/*
 * _Erode.cpp
 *
 *  Created on: March 12, 2019
 *      Author: yankai
 */

#include "_Erode.h"

namespace kai
{

	_Erode::_Erode()
	{
		m_type = vision_erode;
	}

	_Erode::~_Erode()
	{
	}

	bool _Erode::loadConfig(void)
	{
		IF_F(!_VisionBase::loadConfig());
		const json &j = *m_pJ;

		const json *pJF = jK(j, "filters");
		IF__(!pJF || !pJF->is_object(), true);
		const json &jF = *pJF;

		for (auto it = jF.begin(); it != jF.end(); it++)
		{
			const json &Ji = it.value();
			IF_CONT(!Ji.is_object());

			IMG_ERODE e;
			e.init();
			jKv(Ji, "nItr", e.m_nItr);
			jKv(Ji, "kShape", e.m_kShape);
			jKv(Ji, "kW", e.m_kW);
			jKv(Ji, "kH", e.m_kH);
			jKv(Ji, "aX", e.m_aX);
			jKv(Ji, "aY", e.m_aY);
			e.updateKernel();

			m_vFilter.push_back(e);
		}

		return true;
	}

	bool _Erode::link(void)
	{
		IF_F(!this->_VisionBase::link());
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "_VisionBase", n);
		m_pV = (_VisionBase *)(m_pM->findModule(n));
		NULL_F(m_pV);

		return true;
	}

	bool _Erode::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _Erode::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _Erode::filter(void)
	{
		NULL_(m_pV);
		m_pV->copyMatRGB(m_mIn);
		IF_(m_mIn.empty());

		Mat m1 = m_mIn;
		Mat m2;
		Mat *pM1 = &m1;
		Mat *pM2 = &m2;
		Mat *pT;

		for (size_t i = 0; i < m_vFilter.size(); i++)
		{
			IMG_ERODE *pI = &m_vFilter[i];

			cv::erode(*pM1, *pM2,
					  pI->m_kernel,
					  cv::Point(pI->m_aX, pI->m_aY),
					  pI->m_nItr);

			SWAP(pM1, pM2, pT);
		}

		std::lock_guard<std::mutex> lock(m_mutexRGB);
		pM1->copyTo(m_mRGB);
	}

}

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
	}

	_Erode::~_Erode()
	{
	}

	bool _Erode::loadConfig(void)
	{
		IF_F(!_RGBbase::loadConfig());
		const json &j = *m_pJ;

		m_vFilter.clear();
		const json *pJF = jK(j, "filters");
		IF__(!pJF || !pJF->is_object(), true);
		const json &jF = *pJF;

		for (auto it = jF.begin(); it != jF.end(); it++)
		{
			const json &Ji = it.value();
			IF_CONT(!Ji.is_object());

			IMG_ERODE e;
			e.init();
			e.m_name = it.key();
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

	bool _Erode::saveConfig(bool bExport)
	{
		IF_F(!_RGBbase::saveConfig(false));

		json &j = *m_pJ;
		for (const IMG_ERODE &filter : m_vFilter)
		{
			json &jFilter = j["filters"][filter.m_name];
			jFilter["nItr"] = filter.m_nItr;
			jFilter["kShape"] = filter.m_kShape;
			jFilter["kW"] = filter.m_kW;
			jFilter["kH"] = filter.m_kH;
			jFilter["aX"] = filter.m_aX;
			jFilter["aY"] = filter.m_aY;
		}

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _Erode::link(InstanceMgr *pM)
	{
		IF_F(!this->_RGBbase::link(pM));
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "RGBframeIn", n);
		m_pRGBin = dynamic_cast<RGBframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		NULL_F(m_pRGBin);

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
		NULL_(m_pRGBout);
		NULL_(m_pRGBin);
		Mat mIn;
		const uint64_t tStamp = m_pRGBin->get(mIn);
		IF_(mIn.empty());

		Mat m1;
		Mat m2;
		const Mat *pM1 = &mIn;
		Mat *pM2 = &m1;

		for (size_t i = 0; i < m_vFilter.size(); i++)
		{
			IMG_ERODE *pI = &m_vFilter[i];

			cv::erode(*pM1, *pM2,
					  pI->m_kernel,
					  cv::Point(pI->m_aX, pI->m_aY),
					  pI->m_nItr);

			pM1 = pM2;
			pM2 = (pM2 == &m1) ? &m2 : &m1;
		}

		m_pRGBout->set(*pM1, tStamp);
	}

}

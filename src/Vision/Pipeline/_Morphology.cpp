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
	}

	_Morphology::~_Morphology()
	{
	}

	bool _Morphology::loadConfig(void)
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

			IMG_MORPH m;
			m.init();
			m.m_name = it.key();
			jKv(Ji, "morphOp", m.m_morphOp);
			jKv(Ji, "nItr", m.m_nItr);
			jKv(Ji, "kShape", m.m_kShape);
			jKv(Ji, "kW", m.m_kW);
			jKv(Ji, "kH", m.m_kH);
			jKv(Ji, "aX", m.m_aX);
			jKv(Ji, "aY", m.m_aY);
			m.updateKernel();

			m_vFilter.push_back(m);
		}

		return true;
	}

	bool _Morphology::saveConfig(bool bExport)
	{
		IF_F(!_RGBbase::saveConfig(false));

		json &j = *m_pJ;
		for (const IMG_MORPH &filter : m_vFilter)
		{
			json &jFilter = j["filters"][filter.m_name];
			jFilter["morphOp"] = filter.m_morphOp;
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

	bool _Morphology::link(InstanceMgr *pM)
	{
		IF_F(!this->_RGBbase::link(pM));
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "RGBframeIn", n);
		m_pRGBin = dynamic_cast<RGBframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		NULL_F(m_pRGBin);

		return true;
	}

	bool _Morphology::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _Morphology::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _Morphology::filter(void)
	{
		NULL_(m_pRGB);
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
			IMG_MORPH *pM = &m_vFilter[i];

			cv::morphologyEx(*pM1, *pM2,
							 pM->m_morphOp,
							 pM->m_kernel,
							 cv::Point(pM->m_aX, pM->m_aY),
							 pM->m_nItr);

			pM1 = pM2;
			pM2 = (pM2 == &m1) ? &m2 : &m1;
		}

		m_pRGB->set(*pM1, tStamp);
	}

}

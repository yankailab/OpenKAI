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
	}

	_Threshold::~_Threshold()
	{
	}

	bool _Threshold::loadConfig(void)
	{
		IF_F(!_VisionBase::loadConfig());
		const json &j = *m_pJ;

		m_vFilter.clear();
		const json *pJF = jK(j, "filters");
		IF__(!pJF || !pJF->is_object(), true);
		const json &jF = *pJF;

		for (auto it = jF.begin(); it != jF.end(); it++)
		{
			const json &Ji = it.value();
			IF_CONT(!Ji.is_object());

			IMG_THRESHOLD t;
			t.init();
			t.m_name = it.key();
			jKv(Ji, "type", t.m_type);
			jKv(Ji, "vMax", t.m_vMax);
			jKv(Ji, "bAutoThr", t.m_bAutoThr);
			jKv(Ji, "thr", t.m_thr);
			jKv(Ji, "method", t.m_method);
			jKv(Ji, "thrType", t.m_thrType);
			jKv(Ji, "blockSize", t.m_blockSize);
			jKv(Ji, "C", t.m_C);

			m_vFilter.push_back(t);
		}

		return true;
	}

	bool _Threshold::saveConfig(bool bExport)
	{
		IF_F(!_VisionBase::saveConfig(false));

		json &j = *m_pJ;
		for (const IMG_THRESHOLD &filter : m_vFilter)
		{
			json &jFilter = j["filters"][filter.m_name];
			jFilter["type"] = filter.m_type;
			jFilter["vMax"] = filter.m_vMax;
			jFilter["bAutoThr"] = filter.m_bAutoThr;
			jFilter["thr"] = filter.m_thr;
			jFilter["method"] = filter.m_method;
			jFilter["thrType"] = filter.m_thrType;
			jFilter["blockSize"] = filter.m_blockSize;
			jFilter["C"] = filter.m_C;
		}

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _Threshold::link(void)
	{
		IF_F(!this->_VisionBase::link());
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "_VisionBase", n);
		m_pV = (_VisionBase *)(m_pM->findModule(n));
		NULL_F(m_pV);

		return true;
	}

	bool _Threshold::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _Threshold::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _Threshold::filter(void)
	{
		NULL_(m_pV);
		Mat mIn;
		m_pV->copyMatRGB(mIn);
		IF_(mIn.empty());

		if (mIn.type() != CV_8UC1)
			cv::cvtColor(mIn, m_mIn, COLOR_RGB2GRAY);
		else
			m_mIn = mIn;

		Mat m1 = m_mIn;
		Mat m2;
		Mat *pM1 = &m1;
		Mat *pM2 = &m2;
		Mat *pT;

		for (size_t i = 0; i < m_vFilter.size(); i++)
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

		std::lock_guard<std::mutex> lock(m_mutexRGB);
		pM1->copyTo(m_mRGB);
	}

}

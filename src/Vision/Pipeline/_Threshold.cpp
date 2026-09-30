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
	}

	_Threshold::~_Threshold()
	{
	}

	bool _Threshold::loadConfig(void)
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
		IF_F(!_RGBbase::saveConfig(false));

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

	bool _Threshold::link(InstanceMgr *pM)
	{
		IF_F(!this->_RGBbase::link(pM));
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "RGBframeIn", n);
		m_pRGBin = dynamic_cast<RGBframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		NULL_F(m_pRGBin);

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
		NULL_(m_pRGBout);
		NULL_(m_pRGBin);
		
		Mat mIn;
		const uint64_t tStamp = m_pRGBin->get(mIn);
		IF_(mIn.empty());

		Mat mGray;
		const Mat *pM1 = &mIn;
		if (mIn.type() != CV_8UC1)
		{
			cv::cvtColor(mIn, mGray, COLOR_RGB2GRAY);
			pM1 = &mGray;
		}

		Mat m1;
		Mat m2;
		Mat *pM2 = &m1;

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

			pM1 = pM2;
			pM2 = (pM2 == &m1) ? &m2 : &m1;
		}

		m_pRGBout->set(*pM1, tStamp);
	}

}

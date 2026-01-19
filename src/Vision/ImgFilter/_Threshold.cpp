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
		m_pV = nullptr;
	}

	_Threshold::~_Threshold()
	{
	}

	bool _Threshold::init(const json &j)
	{
		IF_F(!_VisionBase::init(j));

		const json &jF = j.at("filters");
		IF__(!jF.is_object(), true);

		for (auto it = jF.begin(); it != jF.end(); it++)
		{
			const json &Ji = it.value();
			IF_CONT(!Ji.is_object());

			IMG_THRESHOLD t;
			t.init();
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

	bool _Threshold::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_VisionBase::link(j, pM));

		string n = "";
		jKv(j, "_VisionBase", n);
		m_pV = (_VisionBase *)(pM->findModule(n));
		NULL_F(m_pV);

		return true;
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
			m_pT->autoFPS();

			filter();
		}
	}

	void _Threshold::filter(void)
	{
		NULL_(m_pV);
		Frame *pF = m_pV->getFrameRGB();
		NULL_(pF);
		IF_(pF->bEmpty());
		IF_(m_fIn.tStamp() >= pF->tStamp());

		if (pF->m()->type() != CV_8UC1)
			m_fIn.copy(pF->cvtColor(COLOR_RGB2GRAY));
		else
			m_fIn.copy(*pF);

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

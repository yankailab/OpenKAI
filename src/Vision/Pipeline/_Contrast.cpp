/*
 * _Contrast.cpp
 *
 *  Created on: March 12, 2019
 *      Author: yankai
 */

#include "_Contrast.h"

namespace kai
{

	_Contrast::_Contrast()
	{
	}

	_Contrast::~_Contrast()
	{
	}

	bool _Contrast::loadConfig(void)
	{
		IF_F(!_RGBbase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "alpha", m_alpha);
		jKv(j, "beta", m_beta);

		return true;
	}

	bool _Contrast::saveConfig(bool bExport)
	{
		IF_F(!_RGBbase::saveConfig(false));

		json &j = *m_pJ;
		j["alpha"] = m_alpha;
		j["beta"] = m_beta;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _Contrast::link(InstanceMgr *pM)
	{
		IF_F(!this->_RGBbase::link(pM));
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "RGBframeIn", n);
		m_pRGBin = dynamic_cast<RGBframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		NULL_F(m_pRGBin);

		return true;
	}

	bool _Contrast::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _Contrast::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _Contrast::filter(void)
	{
		Mat mOut;
		NULL_(m_pRGBout);
		NULL_(m_pRGBin);
		Mat mIn;
		const uint64_t tStamp = m_pRGBin->get(mIn);
		IF_(mIn.empty());

		mIn.convertTo(mOut, -1, m_alpha, m_beta);
		m_pRGBout->set(mOut, tStamp);
	}

}

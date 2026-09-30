/*
 * _Invert.cpp
 *
 *  Created on: March 14, 2019
 *      Author: yankai
 */

#include "_Invert.h"

namespace kai
{

	_Invert::_Invert()
	{
	}

	_Invert::~_Invert()
	{
	}

	bool _Invert::loadConfig(void)
	{
		IF_F(!_RGBbase::loadConfig());

		return true;
	}

	bool _Invert::saveConfig(bool bExport)
	{
		IF_F(!_RGBbase::saveConfig(false));

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _Invert::link(InstanceMgr *pM)
	{
		IF_F(!this->_RGBbase::link(pM));
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "RGBframeIn", n);
		m_pRGBin = dynamic_cast<RGBframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		NULL_F(m_pRGBin);

		return true;
	}

	bool _Invert::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _Invert::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _Invert::filter(void)
	{
		Mat mOut;
		NULL_(m_pRGBout);
		NULL_(m_pRGBin);
		Mat mIn;
		const uint64_t tStamp = m_pRGBin->get(mIn);
		IF_(mIn.empty());

		cv::bitwise_not(mIn, mOut);
		m_pRGBout->set(mOut, tStamp);
	}

}

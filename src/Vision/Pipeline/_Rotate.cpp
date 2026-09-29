/*
 * _Rotate.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_Rotate.h"

namespace kai
{

	_Rotate::_Rotate()
	{
	}

	_Rotate::~_Rotate()
	{
	}

	bool _Rotate::loadConfig(void)
	{
		IF_F(!_RGBbase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "code", m_code);

		return true;
	}

	bool _Rotate::saveConfig(bool bExport)
	{
		IF_F(!_RGBbase::saveConfig(false));

		json &j = *m_pJ;
		j["code"] = m_code;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _Rotate::link(InstanceMgr *pM)
	{
		IF_F(!this->_RGBbase::link(pM));
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "RGBframeIn", n);
		m_pRGBin = dynamic_cast<RGBframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		NULL_F(m_pRGBin);

		return true;
	}

	bool _Rotate::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _Rotate::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _Rotate::filter(void)
	{
		Mat mOut;
		NULL_(m_pRGB);
		NULL_(m_pRGBin);
		Mat mIn;
		const uint64_t tStamp = m_pRGBin->get(mIn);
		IF_(mIn.empty());

		cv::rotate(mIn, mOut, m_code);
		m_pRGB->set(mOut, tStamp);
	}

}

/*
 * _Resize.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_Resize.h"

namespace kai
{

	_Resize::_Resize()
	{
	}

	_Resize::~_Resize()
	{
	}

	bool _Resize::loadConfig(void)
	{
		IF_F(!_RGBbase::loadConfig());

		return true;
	}

	bool _Resize::saveConfig(bool bExport)
	{
		IF_F(!_RGBbase::saveConfig(false));

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _Resize::link(InstanceMgr *pM)
	{
		IF_F(!this->_RGBbase::link(pM));
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "RGBframeIn", n);
		m_pRGBin = dynamic_cast<RGBframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		NULL_F(m_pRGBin);

		return true;
	}

	bool _Resize::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _Resize::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _Resize::filter(void)
	{
		Mat mOut;
		NULL_(m_pRGB);
		NULL_(m_pRGBin);
		Mat mIn;
		const uint64_t tStamp = m_pRGBin->get(mIn);
		IF_(mIn.empty());

		cv::resize(mIn, mOut, cv::Size(m_vSizeRGB.x(), m_vSizeRGB.y()));
		m_pRGB->set(mOut, tStamp);
	}

}

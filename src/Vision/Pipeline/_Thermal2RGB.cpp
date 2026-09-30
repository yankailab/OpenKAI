/*
 * _Thermal2RGB.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_Thermal2RGB.h"

namespace kai
{

	_Thermal2RGB::_Thermal2RGB()
	{
		m_vTrange = Vector2f(0, 40);
	}

	_Thermal2RGB::~_Thermal2RGB()
	{
	}

	bool _Thermal2RGB::loadConfig(void)
	{
		IF_F(!_RGBbase::loadConfig());
		const json &j = *m_pJ;

		jKv<float>(j, "vTrange", m_vTrange);

		return true;
	}

	bool _Thermal2RGB::saveConfig(bool bExport)
	{
		IF_F(!_RGBbase::saveConfig(false));

		json &j = *m_pJ;
		j["vTrange"] = {m_vTrange.x(), m_vTrange.y()};

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _Thermal2RGB::link(InstanceMgr *pM)
	{
		IF_F(!this->_RGBbase::link(pM));
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "RGBframeIn", n);
		m_pRGBin = dynamic_cast<RGBframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		NULL_F(m_pRGBin);

		return true;
	}

	bool _Thermal2RGB::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _Thermal2RGB::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _Thermal2RGB::filter(void)
	{
		Mat mOut;
		NULL_(m_pRGBout);
		NULL_(m_pRGBin);
		Mat mT;
		const uint64_t tStamp = m_pRGBin->get(mT);
		IF_(mT.empty());
		IF_(mT.type() != CV_32FC1);

		Mat mClip;
		cv::min(cv::max(mT, m_vTrange.x()), m_vTrange.y(), mClip);

		Mat mGray;
		float tR = m_vTrange.y() - m_vTrange.x();
		mClip.convertTo(mGray, CV_8UC1, 255.0 / tR, -m_vTrange.x() * 255.0 / tR);

		cv::applyColorMap(mGray, mOut, cv::COLORMAP_JET);
		m_pRGBout->set(mOut, tStamp);
	}

	void _Thermal2RGB::console(const json &j, void *pJSONbase)
	{
		string cmd;
		IF_(!jKv(j, "cmd", cmd));

		if (cmd == "setThermal")
		{
			jKv<float>(j, "vTrange", m_vTrange);
		}

		this->_RGBbase::console(j, pJSONbase);
	}

}

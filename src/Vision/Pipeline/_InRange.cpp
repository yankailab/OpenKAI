/*
 * _InRange.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_InRange.h"

namespace kai
{

	_InRange::_InRange()
	{
		m_vL = Vector3i(0, 0, 0);
		m_vH = Vector3i(255, 255, 255);
	}

	_InRange::~_InRange()
	{
	}

	bool _InRange::loadConfig(void)
	{
		IF_F(!_RGBbase::loadConfig());
		const json &j = *m_pJ;

		jKv<int>(j, "vL", m_vL);
		jKv<int>(j, "vH", m_vH);

		return true;
	}

	bool _InRange::saveConfig(bool bExport)
	{
		IF_F(!_RGBbase::saveConfig(false));

		json &j = *m_pJ;
		j["vL"] = {m_vL.x(), m_vL.y(), m_vL.z()};
		j["vH"] = {m_vH.x(), m_vH.y(), m_vH.z()};

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _InRange::link(InstanceMgr *pM)
	{
		IF_F(!this->_RGBbase::link(pM));
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "RGBframeIn", n);
		m_pRGBin = dynamic_cast<RGBframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		NULL_F(m_pRGBin);

		return true;
	}

	bool _InRange::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _InRange::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _InRange::filter(void)
	{
		Mat mOut;
		NULL_(m_pRGBout);
		NULL_(m_pRGBin);
		Mat mIn;
		const uint64_t tStamp = m_pRGBin->get(mIn);
		IF_(mIn.empty());

		cv::inRange(mIn,
					cv::Scalar(m_vL.x(), m_vL.y(), m_vL.z()),
					cv::Scalar(m_vH.x(), m_vH.y(), m_vH.z()), mOut);
		m_pRGBout->set(mOut, tStamp);
	}

}

/*
 * _Crop.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_Crop.h"

namespace kai
{

	_Crop::_Crop()
	{
		m_vRoi.setZero();
	}

	_Crop::~_Crop()
	{
	}

	bool _Crop::loadConfig(void)
	{
		IF_F(!_RGBbase::loadConfig());
		const json &j = *m_pJ;

		jKv<int>(j, "vRoi", m_vRoi);

		return true;
	}

	bool _Crop::saveConfig(bool bExport)
	{
		IF_F(!_RGBbase::saveConfig(false));

		json &j = *m_pJ;
		j["vRoi"] = {m_vRoi.x(), m_vRoi.y(), m_vRoi.z(), m_vRoi.w()};

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _Crop::link(InstanceMgr *pM)
	{
		IF_F(!this->_RGBbase::link(pM));
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "RGBframeIn", n);
		m_pRGBin = dynamic_cast<RGBframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		NULL_F(m_pRGBin);

		return true;
	}

	bool _Crop::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _Crop::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _Crop::filter(void)
	{
		NULL_(m_pRGBout);
		NULL_(m_pRGBin);
		Mat mIn;
		const uint64_t tStamp = m_pRGBin->get(mIn);
		IF_(mIn.empty());

		Rect r;
		r.x = constrain(m_vRoi.x(), 0, mIn.cols);
		r.y = constrain(m_vRoi.y(), 0, mIn.rows);
		r.width = m_vRoi.z() - r.x;
		r.height = m_vRoi.w() - r.y;

		m_vSizeRGB.x() = r.width;
		m_vSizeRGB.y() = r.height;

		m_pRGBout->set(mIn(r), tStamp);
	}

}

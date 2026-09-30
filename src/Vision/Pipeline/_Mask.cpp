/*
 * _Mask.cpp
 *
 *  Created on: July 2, 2020
 *      Author: yankai
 */

#include "_Mask.h"

namespace kai
{

	_Mask::_Mask()
	{
	}

	_Mask::~_Mask()
	{
	}

	bool _Mask::loadConfig(void)
	{
		IF_F(!_RGBbase::loadConfig());

		return true;
	}

	bool _Mask::saveConfig(bool bExport)
	{
		IF_F(!_RGBbase::saveConfig(false));

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _Mask::link(InstanceMgr *pM)
	{
		IF_F(!this->_RGBbase::link(pM));
		const json &j = *m_pJ;

		string n;
		n = "";
		jKv(j, "RGBframeIn", n);
		m_pRGBin = dynamic_cast<RGBframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		NULL_F(m_pRGBin);

		n = "";
		jKv(j, "RGBframeMaskIn", n);
		m_pMaskin = dynamic_cast<RGBframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		NULL_F(m_pMaskin);

		return true;
	}

	bool _Mask::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _Mask::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _Mask::filter(void)
	{
		NULL_(m_pRGBout);
		NULL_(m_pRGBin);
		NULL_(m_pMaskin);

		Mat mIn;
		const uint64_t tStamp = m_pRGBin->get(mIn);
		IF_(mIn.empty());
		Mat mMask;
		m_pMaskin->get(mMask);
		IF_(mMask.empty());

		Mat mBg;

		mIn.copyTo(mBg, mMask);

		m_pRGBout->set(mBg, tStamp);
	}

}

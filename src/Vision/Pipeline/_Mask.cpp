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
		m_pRGBin = dynamic_cast<RGBframe *>(static_cast<DataStreamBase *>(pM->findDataStream(n)));
		NULL_F(m_pRGBin);

		n = "";
		jKv(j, "RGBframeMask", n);
		m_pMask = dynamic_cast<RGBframe *>(static_cast<DataStreamBase *>(pM->findDataStream(n)));
		NULL_F(m_pMask);

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
		NULL_(m_pRGB);
		NULL_(m_pRGBin);
		NULL_(m_pMask);

		const RGBframe::SnapshotPtr frame = m_pRGBin->get();
		const Mat &mIn = frame->m_mRGB;
		IF_(mIn.empty());
		const RGBframe::SnapshotPtr maskFrame = m_pMask->get();
		const Mat &mMask = maskFrame->m_mRGB;
		IF_(mMask.empty());

		Mat mBg;

		mIn.copyTo(mBg, mMask);

		m_pRGB->set(mBg, frame->m_tStamp);
	}

}

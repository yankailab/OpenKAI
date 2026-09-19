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

	bool _Mask::init(const json &j)
	{
		IF_F(!_VisionBase::init(j));

		return true;
	}

	bool _Mask::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_VisionBase::link(j, pM));

		string n;
		n = "";
		jKv(j, "_VisionBase", n);
		m_pV = (_VisionBase *)(pM->findModule(n));
		NULL_F(m_pV);

		n = "";
		jKv(j, "_VisionBaseMask", n);
		m_pVmask = (_VisionBase *)(pM->findModule(n));
		NULL_F(m_pVmask);

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
		NULL_(m_pV);
		NULL_(m_pVmask);

		Mat *pM = m_pV->getMatRGB();
		NULL_(pM);
		Mat *pMmask = m_pVmask->getMatRGB();
		NULL_(pMmask);

		IF_(pM->empty());
		IF_(pMmask->empty());

		pM->copyTo(m_mIn);
		pMmask->copyTo(m_mMask);

		Mat mV = m_mIn;
		Mat mM = m_mMask;
		Mat mBg;
		// mBg.zeros(mV.rows, mV.cols, mV.type());

		mV.copyTo(mBg, mM);
		mBg.copyTo(m_mRGB);
	}

}

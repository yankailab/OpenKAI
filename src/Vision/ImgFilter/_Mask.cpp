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
		m_pV = nullptr;
		m_pVmask = nullptr;
	}

	_Mask::~_Mask()
	{
	}

	bool _Mask::init(const json& j)
	{
		IF_F(!_VisionBase::init(j));

		return true;
	}

	bool _Mask::link(const json& j, ModuleMgr* pM)
	{
		IF_F(!this->_VisionBase::link(j, pM));


		string n;
		n = "";
		n = j.value("_VisionBase", "");
		m_pV = (_VisionBase *)(pM->findModule(n));
		NULL__(m_pV);

		n = "";
		n = j.value("_VisionBaseMask", "");
		m_pVmask = (_VisionBase *)(pK->parent()->findModule(n));
		NULL__(m_pVmask);

		return true;
	}

	bool _Mask::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _Mask::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _Mask::filter(void)
	{
		NULL_(m_pV);
		NULL_(m_pVmask);

		Frame *pF = m_pV->getFrameRGB();
		NULL_(pF);
		Frame *pFmask = m_pVmask->getFrameRGB();
		NULL_(pFmask);

		IF_(pF->bEmpty());
		IF_(pFmask->bEmpty());
		IF_(m_fIn.tStamp() >= pF->tStamp());

		m_fIn.copy(*pF);
		m_fMask.copy(*pFmask);

		Mat mV = *m_fIn.m();
		Mat mM = *m_fMask.m();
		Mat mBg;
		mBg.zeros(mV.rows, mV.cols, mV.type());

		mV.copyTo(mBg, mM);
		m_fRGB.copy(mBg);
	}

}

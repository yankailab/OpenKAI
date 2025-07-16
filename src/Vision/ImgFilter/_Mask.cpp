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

	int _Mask::init(void *pKiss)
	{
		CHECK_(_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		return OK_OK;
	}

	int _Mask::link(void)
	{
		CHECK_(this->_VisionBase::link());

		Kiss *pK = (Kiss *)m_pKiss;

		string n;
		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->findModule(n));
		NULL__(m_pV, OK_ERR_NOT_FOUND);

		n = "";
		pK->v("_VisionBaseMask", &n);
		m_pVmask = (_VisionBase *)(pK->parent()->findModule(n));
		NULL__(m_pVmask, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	int _Mask::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
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

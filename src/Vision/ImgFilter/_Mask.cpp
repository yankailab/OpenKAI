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
		close();
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
		NULL_(m_pV, OK_ERR_NOT_FOUND);

		n = "";
		pK->v("_VisionBaseMask", &n);
		m_pVmask = (_VisionBase *)(pK->parent()->findModule(n));
		NULL_(m_pVmask, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	bool _Mask::open(void)
	{
		NULL_F(m_pV);
		NULL_F(m_pVmask);

		m_bOpen = m_pV->isOpened();
		IF_F(!m_bOpen);

		m_bOpen = m_pVmask->isOpened();

		return m_bOpen;
	}

	void _Mask::close(void)
	{
		this->_VisionBase::close();
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
			if (!m_bOpen)
				open();

			m_pT->autoFPSfrom();

			if (m_bOpen)
			{
				if (m_fIn.tStamp() < m_pV->getFrameRGB()->tStamp())
				{
					filter();
				}
			}

			m_pT->autoFPSto();
		}
	}

	void _Mask::filter(void)
	{
		IF_(m_pV->getFrameRGB()->bEmpty());
		IF_(m_pVmask->getFrameRGB()->bEmpty());

		m_fIn.copy(*m_pV->getFrameRGB());
		m_fMask.copy(*m_pVmask->getFrameRGB());

		Mat mV = *m_fIn.m();
		Mat mM = *m_fMask.m();
		Mat mBg;
		mBg.zeros(mV.rows, mV.cols, mV.type());

		mV.copyTo(mBg, mM);
		m_fRGB.copy(mBg);
	}

}

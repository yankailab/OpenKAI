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
		m_pV = NULL;
		m_pVmask = NULL;
	}

	_Mask::~_Mask()
	{
		close();
	}

	bool _Mask::init(void *pKiss)
	{
		IF_F(!_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		return true;
	}

	bool _Mask::link(void)
	{
		IF_F(!this->_VisionBase::link());

		Kiss *pK = (Kiss *)m_pKiss;

		string n;
		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->getInst(n));
		IF_Fl(!m_pV, n + ": not found");

		n = "";
		pK->v("_VisionBaseMask", &n);
		m_pVmask = (_VisionBase *)(pK->parent()->getInst(n));
		IF_Fl(!m_pVmask, n + ": not found");

		return true;
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

	bool _Mask::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _Mask::update(void)
	{
		while (m_pT->bRun())
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

/*
 * _Orbbec.cpp
 *
 *  Created on: Feb 13, 2023
 *      Author: yankai
 */

#include "_Orbbec.h"

namespace kai
{

	_Orbbec::_Orbbec()
	{
	}

	_Orbbec::~_Orbbec()
	{
	}

	bool _Orbbec::init(const json &j)
	{
		IF_F(!_RGBDbase::init(j));

		return true;
	}

	bool _Orbbec::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_RGBDbase::link(j, pM));

		return true;
	}

	bool _Orbbec::open(void)
	{
		IF__(m_bOpen, true);

		m_spObConfig = std::make_shared<ob::Config>();
		m_spObConfig->enableVideoStream(OB_STREAM_COLOR);
		m_obPipe.start(m_spObConfig);

		m_bOpen = true;
		return true;
	}

	void _Orbbec::close(void)
	{
		IF_(!m_bOpen);

		m_obPipe.stop();
	}

	bool _Orbbec::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _Orbbec::check(void)
	{
		return this->_RGBDbase::check();
	}

	void _Orbbec::update(void)
	{
		while (m_pT->bAlive())
		{
			if (!m_bOpen)
			{
				if (!open())
				{
					LOG_E("Cannot open Orbbec");
					m_pT->sleepT(SEC_2_USEC);
					continue;
				}
			}

			m_pT->autoFPS();

			if (updateOrbbec())
			{
				m_pTPP->run();
			}
			else
			{
				m_pT->sleepT(SEC_2_USEC);
				m_bOpen = false;
			}
		}
	}

	bool _Orbbec::updateOrbbec(void)
	{
		IF_F(!check());

		// Wait for up to 100ms for a frameset in blocking mode.
		auto obFrameSet = m_obPipe.waitForFrameset();
		NULL_F(obFrameSet);

		// get color frame from frameset.
		auto obColorFrame = obFrameSet->getFrame(OB_FRAME_COLOR);

		// Render colorFrame.
		obColorFrame->getData();

		return true;
	}

#ifdef WITH_3D
	int _Orbbec::getPointCloud(_PCframe *pPCframe, int nPmax)
	{
		NULL__(pPCframe, -1);
		PointCloud *pPC = pPCframe->getNextBuffer();

		return 0;
	}
#endif

}

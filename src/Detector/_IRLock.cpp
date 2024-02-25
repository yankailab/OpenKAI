/*
 * _IRLock.cpp
 *
 *  Created on: Nov 20, 2019
 *      Author: yankai
 */

#include "_IRLock.h"

namespace kai
{

	_IRLock::_IRLock()
	{
		m_pIO = NULL;
		m_iBuf = 0;

		m_vOvCamSize.x = 1.0 / 319.0;
		m_vOvCamSize.y = 1.0 / 199.0;
	}

	_IRLock::~_IRLock()
	{
	}

	bool _IRLock::init(void *pKiss)
	{
		IF_F(!this->_DetectorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		vFloat2 vCamSize;
		if (pK->v("vCamSize", &vCamSize))
		{
			m_vOvCamSize.x = 1.0 / vCamSize.x;
			m_vOvCamSize.y = 1.0 / vCamSize.y;
		}

		return true;
	}

	bool _IRLock::link(void)
	{
		IF_F(!this->_DetectorBase::link());

		Kiss *pK = (Kiss *)m_pKiss;

		string n;
		n = "";
		F_ERROR_F(pK->v("_IObase", &n));
		m_pIO = (_IObase *)(pK->getInst(n));
		NULL_Fl(m_pIO, n + ": not found");

		return true;
	}

	int _IRLock::check(void)
	{
		NULL__(m_pU, -1);
		NULL__(m_pIO, -1);

		return this->_DetectorBase::check();
	}

	bool _IRLock::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _IRLock::update(void)
	{
		while (m_pT->bThread())
		{
			m_pT->autoFPSfrom();

			detect();

			ON_SLEEP;
			m_pT->autoFPSto();
		}
	}

	void _IRLock::detect(void)
	{
		IF_(!readPacket());

		_Object o;
		//	o.m_tStamp = m_pT->getTfrom();
		o.setTopClass(INT_MAX, 1.0);

		uint16_t x = unpack_uint16(&m_pBuf[8], false);
		uint16_t y = unpack_uint16(&m_pBuf[10], false);
		uint16_t w = unpack_uint16(&m_pBuf[12], false);
		uint16_t h = unpack_uint16(&m_pBuf[14], false);
		o.setBB2D(x, y, w, h);

		// TODO:distance

		m_pU->add(o);
		m_pU->swap();
	}

	bool _IRLock::readPacket(void)
	{
		uint8_t inByte;
		int nRead;

		while ((nRead = m_pIO->read(&inByte, 1)) > 0)
		{
			if (m_iBuf > 1)
			{
				m_pBuf[m_iBuf] = inByte;
				m_iBuf++;

				if (m_iBuf >= IRLOCK_N_BLOCK)
				{
					m_iBuf = 0;
					return true;
				}
			}
			else if (m_iBuf == 0 && inByte == IRLOCK_SYNC_L)
			{
				m_pBuf[0] = inByte;
				m_iBuf++;
			}
			else if (m_iBuf == 1)
			{
				if (inByte == IRLOCK_SYNC_H)
				{
					m_pBuf[1] = inByte;
					m_iBuf++;
				}
				else
				{
					m_iBuf = 0;
				}
			}
		}

		return false;
	}

	void _IRLock::console(void *pConsole)
	{
		NULL_(pConsole);
		IF_(check() < 0);
		this->_DetectorBase::console(pConsole);

		string msg = "| ";
		_Object *pO;
		int i = 0;
		while ((pO = m_pU->get(i++)) != NULL)
		{
			msg += f2str(pO->getPos().z) + " | ";
		}

		((_Console *)pConsole)->addMsg(msg, 1);
	}

}

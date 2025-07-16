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
		m_pIO = nullptr;
		m_iBuf = 0;

		m_vOvCamSize.x = 1.0 / 319.0;
		m_vOvCamSize.y = 1.0 / 199.0;
	}

	_IRLock::~_IRLock()
	{
	}

	int _IRLock::init(void *pKiss)
	{
		CHECK_(this->_DetectorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		vFloat2 vCamSize;
		if (pK->v("vCamSize", &vCamSize))
		{
			m_vOvCamSize.x = 1.0 / vCamSize.x;
			m_vOvCamSize.y = 1.0 / vCamSize.y;
		}

		return OK_OK;
	}

	int _IRLock::link(void)
	{
		CHECK_(this->_DetectorBase::link());

		Kiss *pK = (Kiss *)m_pKiss;

		string n;
		n = "";
		pK->v("_IObase", &n);
		m_pIO = (_IObase *)(pK->findModule(n));
		NULL__(m_pIO, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	int _IRLock::check(void)
	{
		NULL__(m_pU, OK_ERR_NULLPTR);
		NULL__(m_pIO, OK_ERR_NULLPTR);

		return this->_DetectorBase::check();
	}

	int _IRLock::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	void _IRLock::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			detect();

			ON_PAUSE;
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
		o.setRect(x, y, w, h);

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
		IF_(check() != OK_OK);
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

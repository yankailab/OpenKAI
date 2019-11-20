/*
 * _IRLock.cpp
 *
 *  Created on: Nov 20, 2019
 *      Author: yankai
 */

#include "_IRLock.h"

#ifdef USE_OPENCV_CONTRIB

namespace kai
{

_IRLock::_IRLock()
{
	m_pIO = NULL;
	m_iBuf = 0;

}

_IRLock::~_IRLock()
{
}

bool _IRLock::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	string iName;
	iName = "";
	F_ERROR_F(pK->v("_IOBase", &iName));
	m_pIO = (_IOBase*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pIO, iName + ": not found");

	return true;
}

bool _IRLock::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _IRLock::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();
		updateObj();

		if(m_bGoSleep)
		{
			m_pPrev->reset();
		}

		this->autoFPSto();
	}
}

bool _IRLock::readPacket(void)
{
	uint8_t	inByte;
	int		nRead;

	while ((nRead = m_pIO->read(&inByte,1)) > 0)
	{
		if (m_iBuf > 1)
		{
			m_pBuf[m_iBuf] = inByte;
			m_iBuf++;

			if (m_iBuf >= IRLOCK_N_PACKET)
				return true;
		}
		else if (m_iBuf == 0 && inByte == IRLOCK_SYNC_L)
		{
			m_pBuf[0] = inByte;
			m_iBuf++;
		}
		else if (m_iBuf == 1)
		{
			if(inByte == IRLOCK_SYNC_H)
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

void _IRLock::detect(void)
{
	IF_(!readPacket());

	OBJECT o;
	o.init();
	o.m_tStamp = m_tStamp;

	uint16_t x = *((uint16_t*)&m_pBuf[6]);
	uint16_t y = *((uint16_t*)&m_pBuf[8]);
	uint16_t w = *((uint16_t*)&m_pBuf[10]);
	uint16_t h = *((uint16_t*)&m_pBuf[12]);

	add(&o);

	m_iBuf = 0;
}

void _IRLock::draw(void)
{
	this->_DetectorBase::draw();

	string msg;
	msg = "nMarker: " + i2str(this->size());
	addMsg(msg);
	IF_(this->size() <= 0);
}

}
#endif

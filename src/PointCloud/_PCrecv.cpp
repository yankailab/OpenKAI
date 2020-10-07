/*
 * _PCio.cpp
 *
 *  Created on: Oct 8, 2020
 *      Author: yankai
 */

#include "_PCsend.h"

#ifdef USE_OPENCV
#ifdef USE_OPEN3D

namespace kai
{

_PCsend::_PCsend()
{
	m_pPCB = NULL;
	m_pIO = NULL;
}

_PCsend::~_PCsend()
{
}

bool _PCsend::init(void *pKiss)
{
	IF_F(!_PCbase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	string n;

	n = "";
	F_ERROR_F(pK->v("_PCbase", &n));
	m_pPCB = (_PCbase*) (pK->getInst(n));
	NULL_Fl(m_pPCB, "_PCbase not found");

	n = "";
	F_ERROR_F(pK->v("_IOBase", &n));
	m_pIO = (_IOBase*) (pK->getInst(n));
	NULL_Fl(m_pIO, "_IOBase not found");

	return true;
}

bool _PCsend::start(void)
{
	IF_F(!this->_ThreadBase::start());

	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _PCsend::check(void)
{
	NULL__(m_pPCB, -1);
	NULL__(m_pIO, -1);

	return 0;
}

void _PCsend::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateIO();
		m_sPC.update();

		this->autoFPSto();
	}
}

void _PCsend::updateIO(void)
{
	IF_(check()<0);

	*m_sPC.next() = *m_pPCB->getPC();
}

}
#endif
#endif

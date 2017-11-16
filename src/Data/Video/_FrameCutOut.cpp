/*
 * _FrameCutOut.cpp
 *
 *  Created on: Nov 16, 2017
 *      Author: yankai
 */

#include "_FrameCutOut.h"

namespace kai
{

_FrameCutOut::_FrameCutOut()
{
	m_progress = 0.0;
	m_pFrameIn = NULL;
	m_pFrameOut = NULL;
}

_FrameCutOut::~_FrameCutOut()
{
}

bool _FrameCutOut::init(void* pKiss)
{
	IF_F(!this->_DataBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

//	KISSm(pK, );

	m_pFrameIn = new Frame();
	m_pFrameOut = new Frame();

	return true;
}

bool _FrameCutOut::link(void)
{
	IF_F(!this->_DataBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	return true;
}

bool _FrameCutOut::start(void)
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

void _FrameCutOut::update(void)
{
	srand(time(NULL));

}

}


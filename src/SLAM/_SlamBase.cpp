/*
 * _SlamBase.cpp
 *
 *  Created on: May 29, 2018
 *      Author: yankai
 */

#include "_SlamBase.h"

namespace kai
{

_SlamBase::_SlamBase()
{
	m_bReady = false;
	reset();
}

_SlamBase::~_SlamBase()
{
}

bool _SlamBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	return true;
}

void _SlamBase::detect(void)
{
}

bool _SlamBase::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;

	string msg;
	msg = "pos=(" + f2str(m_pos.x) + ", "
				  + f2str(m_pos.y) + ", "
				  + f2str(m_pos.z) + ")";

	pWin->tabNext();
	pWin->addMsg(&msg);
	pWin->tabPrev();

	return true;
}

bool _SlamBase::console(int& iY)
{
	IF_F(!this->_ThreadBase::console(iY));

	string msg;
	msg = "pos=(" + f2str(m_pos.x) + ", "
				  + f2str(m_pos.y) + ", "
				  + f2str(m_pos.z) + ")";
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

void _SlamBase::reset(void)
{
	m_pos.init();
}

}

/*
 * _ANR.cpp
 *
 *  Created on: Feb 26, 2019
 *      Author: yankai
 */

#include "_ANR.h"

namespace kai
{

_ANR::_ANR()
{
	m_pD = NULL;
	m_cnPrefix = "";
	m_cnPos = 0.05;
	m_cn = "";

}

_ANR::~_ANR()
{
}

bool _ANR::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,cnPrefix);
	KISSm(pK,cnPos);

	string iName;

	iName = "";
	F_ERROR_F(pK->v("_DetectorBase", &iName));
	m_pD = (_DetectorBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pD, iName + " not found");

	return true;
}

bool _ANR::start(void)
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

void _ANR::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		cn();
//		lp();

		this->autoFPSto();
	}
}

int _ANR::check(void)
{
	NULL__(m_pD,-1);

	return 0;
}

void _ANR::cn(void)
{
	IF_(check()<0);

	OBJECT* pO;
	int i = 0;
	while ((pO = m_pD->at(i++)) != NULL)
	{
		vFloat4 bb = pO->m_bb;
		string s = string(pO->m_pText);

		std::string::size_type pos = s.find(m_cnPrefix);
		IF_CONT(pos == std::string::npos);

		m_cnBB = pO->m_bb;
		break;
	}

	i=0;
	while ((pO = m_pD->at(i++)) != NULL)
	{
		vFloat4 bb = pO->m_bb;

		IF_CONT(bb.x < m_cnBB.z);
		IF_CONT(abs(bb.y - m_cnBB.y) > m_cnPos);
		IF_CONT(abs(bb.w - m_cnBB.w) > m_cnPos);

		string s = string(pO->m_pText);
		IF_CONT(s.length() < 6);

		m_cn = m_cnPrefix + s.substr(0,6);
	}

	LOG_I("Number: " + m_cn);
}

void _ANR::lp(void)
{
	IF_(check()<0);

	LOG_I("Number: ");
}

bool _ANR::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	string msg;

	return true;
}

bool _ANR::console(int& iY)
{
	IF_F(!this->_ThreadBase::console(iY));

	string msg;

	C_MSG("Number: ");
	return true;
}

}

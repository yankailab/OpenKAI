/*
 *  Created on: June 21, 2019
 *      Author: yankai
 */
#include "_Universe.h"

namespace kai
{

_Universe::_Universe()
{
	m_minConfidence = 0.0;
	m_minArea = -1.0;
	m_maxArea = -1.0;
	m_minW = -1.0;
	m_maxW = -1.0;
	m_minH = -1.0;
	m_maxH = -1.0;
	m_bbScale = -1.0;

	m_bDrawStatistics = false;
	m_bDrawClass = false;

	clearObj();
}

_Universe::~_Universe()
{
}

bool _Universe::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	//general
	KISSm(pK, minConfidence);
	KISSm(pK, minArea);
	KISSm(pK, maxArea);
	KISSm(pK, minW);
	KISSm(pK, minH);
	KISSm(pK, maxW);
	KISSm(pK, maxH);
	KISSm(pK, bbScale);

	clearObj();

	//draw
	KISSm(pK, bDrawStatistics);
	KISSm(pK, bDrawClass);

	return true;
}

void _Universe::clearObj(void)
{
	m_iSwitch = 0;
	updateObj();
	m_pPrev->reset();
	m_pNext->reset();
}

void _Universe::updateObj(void)
{
	m_iSwitch = 1 - m_iSwitch;
	m_pPrev = &m_pO[m_iSwitch];
	m_pNext = &m_pO[1 - m_iSwitch];
	m_pNext->reset();
}

bool _Universe::start(void)
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

void _Universe::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->autoFPSto();
	}
}

_Object* _Universe::get(int i)
{
	return m_pPrev->at(i);
}

int _Universe::add(_Object* pO)
{
	NULL__(pO, -1);

	return m_pNext->add(pO);
}

void _Universe::pipeIn(void)
{
}

bool _Universe::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();
	Mat* pMat = pFrame->m();
	IF_F(pMat->empty());

	return true;
}

bool _Universe::console(int& iY)
{
	IF_F(!this->_ThreadBase::console(iY));

	string msg;

	return true;
}

}

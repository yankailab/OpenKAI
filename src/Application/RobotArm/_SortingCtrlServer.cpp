/*
 * _SortingImg.cpp
 *
 *  Created on: May 28, 2019
 *      Author: yankai
 */

#include "_SortingCtrlServer.h"

#ifdef USE_OPENCV

namespace kai
{

_SortingCtrlServer::_SortingCtrlServer()
{
	m_pDV = NULL;
	m_pPB = NULL;
	m_cSpeed = 0.0;
	m_cLen = 2.0;

	m_newO.init();
	m_bbSize = 0.05;
	m_dT = 0.0;
	m_iState = SORT_STATE_OFF;
	m_ieState.init(USEC_1SEC);
}

_SortingCtrlServer::~_SortingCtrlServer()
{
}

bool _SortingCtrlServer::init(void *pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	pK->v("cSpeed", &m_cSpeed);
	pK->v("cLen", &m_cLen);
	pK->v("bbSize", &m_bbSize);
	pK->v("dT", &m_dT);
	pK->v("tStateInterval", &m_ieState.m_tInterval);

	string n;

	n = "";
	F_ERROR_F(pK->v("_DepthVisionBase", &n));
	m_pDV = (_DepthVisionBase*) (pK->getInst(n));
	IF_Fl(!m_pDV, n + " not found");

	m_nClass = 5;

	n = "";
	F_ERROR_F(pK->v("_ProtocolBase", &n));
	m_pPB = (_ProtocolBase*) (pK->getInst(n));
	IF_Fl(!m_pPB, n + " not found");

	m_pPB->setCallback(callbackCMD, this);

	return true;
}

bool _SortingCtrlServer::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

int _SortingCtrlServer::check(void)
{
	NULL__(m_pPB, -1);
	NULL__(m_pDV, -1);

	return 0;
}

void _SortingCtrlServer::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		if (check() >= 0)
		{
			updateImg();
			m_pU->updateObj();

//		if(m_ieState.update(m_pT->getTstamp()))
//			m_pPB->sendState(m_iState);

			if (m_pT->bGoSleep())
				m_pU->m_pPrev->clear();
		}

		m_pT->autoFPSto();
	}
}

void _SortingCtrlServer::updateImg(void)
{
	IF_(check() < 0);

	//update existing target positions
	float spd = m_cSpeed * ((float) m_pT->getDtime()) * 1e-6;
	int i = 0;
	_Object *pO;
	while ((pO = m_pU->get(i++)))
	{
		pO->setY(pO->getY() + spd);
		IF_CONT(pO->getY() > m_cLen);

		m_pU->add(*pO);
	}

	if (m_newO.getTopClass() >= 0)
	{
		m_pU->add(m_newO);
		m_newO.init();
	}
}

void _SortingCtrlServer::handleCMD(uint8_t *pCMD)
{
	NULL_(pCMD);
	IF_(check() < 0);

	uint8_t cmd = pCMD[1];

	if (cmd == SORTINGCTRL_OBJ)
	{
		float d = m_cSpeed * ((float) m_dT) * 1e-6;

		m_newO.setX((float) pCMD[3] / 255.0);
		m_newO.setY((float) pCMD[3] / 255.0);
		m_newO.setWidth(m_bbSize * 2);
		m_newO.setHeight(m_bbSize * 2);
		IF_(m_newO.area() < m_pU->m_rArea.x);

		vFloat4 bb = m_newO.getBB2D();
		m_newO.setZ(m_pDV->d(&bb));
		m_newO.setTopClass(unpack_int16(&pCMD[5], false), 1.0);
	}
	else if (cmd == SORTINGCTRL_STATE)
	{
		m_iState = pCMD[3];
	}
}

void _SortingCtrlServer::draw(void)
{
	this->_DetectorBase::draw();
}

}
#endif

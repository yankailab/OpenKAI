/*
 * _SortingImg.cpp
 *
 *  Created on: May 28, 2019
 *      Author: yankai
 */

#include "_SortingCtrlServer.h"

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

	string iName;

	iName = "";
	F_ERROR_F(pK->v("_DepthVisionBase", &iName));
	m_pDV = (_DepthVisionBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pDV, iName + " not found");

	m_nClass = 5;

	iName = "";
	F_ERROR_F(pK->v("_ProtocolBase", &iName));
	m_pPB = (_ProtocolBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pPB, iName + " not found");

	m_pPB->setCallback(callbackCMD,this);

	return true;
}

bool _SortingCtrlServer::start(void)
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

int _SortingCtrlServer::check(void)
{
	NULL__(m_pPB, -1);
	NULL__(m_pDV, -1);

	return 0;
}

void _SortingCtrlServer::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateImg();
		updateObj();

//		if(m_ieState.update(m_tStamp))
//			m_pPB->sendState(m_iState);

		if (m_bGoSleep)
		{
			m_pPrev->reset();
		}

		this->autoFPSto();
	}
}

void _SortingCtrlServer::updateImg(void)
{
	IF_(check() < 0);

	//update existing target positions
	float spd = m_cSpeed * ((float) m_dTime) * 1e-6;
	int i = 0;
	OBJECT *pO;
	while ((pO = at(i++)))
	{
		pO->m_bb.y += spd;
		pO->m_bb.w += spd;
		IF_CONT(pO->m_bb.midY() > m_cLen);

		add(pO);
	}

	if(m_newO.m_topClass >= 0)
	{
		add(&m_newO);
		m_newO.init();
	}
}

void _SortingCtrlServer::handleCMD(uint8_t* pCMD)
{
	NULL_(pCMD);
	IF_(check() < 0);

	uint8_t cmd = pCMD[1];

	if(cmd == SORTINGCTRL_OBJ)
	{
		float d = m_cSpeed * ((float) m_dT) * 1e-6;
		//		m_newO.m_bb.x = constrain(((float)unpack_uint16(&pCMD[3], false))*0.001 - m_bbSize, 0.0, 1.0);
		//		m_newO.m_bb.y = constrain(((float)unpack_uint16(&pCMD[5], false))*0.001 - m_bbSize + d, 0.0, 1.0);
		m_newO.m_bb.x = (float)pCMD[3]/255.0 - m_bbSize;
		m_newO.m_bb.y = (float)pCMD[4]/255.0 - m_bbSize + d;
		m_newO.m_bb.z = constrain<float>(m_newO.m_bb.x + m_bbSize * 2, 0.0, 1.0);
		m_newO.m_bb.w = constrain<float>(m_newO.m_bb.y + m_bbSize * 2 + d, 0.0, 1.0);
		IF_(m_newO.m_bb.area() < m_minArea);

		m_newO.m_dist = m_pDV->d(&m_newO.m_bb);
		m_newO.m_topClass = unpack_int16(&pCMD[5], false);
	}
	else if(cmd == SORTINGCTRL_STATE)
	{
		m_iState = pCMD[3];
	}
}

bool _SortingCtrlServer::draw(void)
{
	IF_F(!this->_DetectorBase::draw());
	Window *pWin = (Window*) this->m_pWindow;
	Mat *pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());

	return true;
}

bool _SortingCtrlServer::console(int &iY)
{
	IF_F(!this->_DetectorBase::console(iY));

	string msg;

	return true;
}

}

#include "Rover_move.h"

namespace kai
{

Rover_move::Rover_move()
{
	m_pR = NULL;
	m_pMavlink = NULL;
	m_pDet = NULL;
	m_hdg = 0.0;
	m_nSpeed = 0.0;

	m_iTag = -1;
	m_tagRoi.init();
	m_tagRoi.z = 1.0;
	m_tagRoi.w = 1.0;
	m_tagAngle = 0;
	m_iPinLEDtag = 21;
}

Rover_move::~Rover_move()
{
}

bool Rover_move::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	KISSm(pK,nSpeed);
	KISSm(pK,iPinLEDtag);

	pK->v("tagX",&m_tagRoi.x);
	pK->v("tagY",&m_tagRoi.y);
	pK->v("tagZ",&m_tagRoi.z);
	pK->v("tagW",&m_tagRoi.w);

	string iName;
	iName = "";
	F_ERROR_F(pK->v("Rover_base", &iName));
	m_pR = (Rover_base*) (pK->parent()->getChildInst(iName));
	NULL_Fl(m_pR, iName+": not found");

	iName = "";
	F_ERROR_F(pK->v("_Mavlink", &iName));
	m_pMavlink = (_Mavlink*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pMavlink, iName+": not found");

	iName = "";
	F_ERROR_F(pK->v("_DetectorBase", &iName));
	m_pDet = (_DetectorBase*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pDet, iName+": not found");

	return true;
}

int Rover_move::check(void)
{
	NULL__(m_pR, -1);
	NULL__(m_pR->m_pCMD, -1);
	NULL__(m_pMavlink, -1);
	NULL__(m_pDet, -1);

	return 0;
}

void Rover_move::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);
	if(!bActive())
	{
		return;
	}

	float nSpeed;
	switch (m_pR->m_mode)
	{
	case rover_forward:
		nSpeed = m_nSpeed;
		break;
	case rover_backward:
		nSpeed = -m_nSpeed;
		break;
	default:
		nSpeed = 0.0;
		break;
	}

	string mission = m_pMC->getCurrentMissionName();
	if(mission == "IDLE")
	{
		if(m_pR->m_mode != rover_idle && m_pR->m_mode != rover_manual)
			m_pMC->transit("MOVE");
	}
	else if(mission == "MOVE")
	{
		m_pR->setSpeed(nSpeed);
	}
	else if(mission == "TAG")
	{
		m_pR->setSpeed(0.0);
	}

	if(m_tStamp - m_pMavlink->m_msg.time_stamps.global_position_int > USEC_1SEC)
	{
		m_pMavlink->requestDataStream(MAV_DATA_STREAM_POSITION, 5);
		return;
	}

	m_hdg = ((float)(m_pMavlink->m_msg.global_position_int.hdg)) * 1e-2;
	m_pR->setHdg(m_hdg);
	if(m_pR->m_targetHdg < 0.0)
		m_pR->setTargetHdg(m_hdg);

	findTag();
}

bool Rover_move::findTag(void)
{
	IF__(check()<0, false);

	if(m_pDet->size()>0)
		m_pR->setLED(m_iPinLEDtag,1);
	else
		m_pR->setLED(m_iPinLEDtag,0);

	OBJECT* pO;
	int i=0;
	while((pO = m_pDet->at(i++)) != NULL)
	{
		IF_CONT(pO->m_c.x < m_tagRoi.x);
		IF_CONT(pO->m_c.x > m_tagRoi.z);
		IF_CONT(pO->m_c.y < m_tagRoi.y);
		IF_CONT(pO->m_c.y > m_tagRoi.w);

		m_tagAngle = pO->m_angle;
		m_pR->setTargetHdg(m_hdg + m_tagAngle);

		IF_CONT(pO->m_topClass == m_iTag);

		m_iTag = pO->m_topClass;
		m_pMC->transit("TAG");

		return true;
	}

	return false;
}

bool Rover_move::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	NULL_F(pMat);
	IF_F(pMat->empty());

	string msg = *this->getName()
			+ ": hdg=" + f2str(m_hdg)
			+ ", nSpeed=" + f2str(m_nSpeed);
	pWin->addMsg(msg);

	return true;
}

bool Rover_move::console(int& iY)
{
	IF_F(!this->ActionBase::console(iY));
	IF_F(check()<0);

	string msg;
	C_MSG("hdg=" + f2str(m_hdg) + ", nSpeed=" + f2str(m_nSpeed));

	return true;
}

}

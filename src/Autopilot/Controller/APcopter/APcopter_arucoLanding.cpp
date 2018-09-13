#include "APcopter_arucoLanding.h"

namespace kai
{

APcopter_arucoLanding::APcopter_arucoLanding()
{
	m_pAP = NULL;
	m_pArUco = NULL;
	m_pDV = NULL;

	m_bLocked = false;
	m_orientation.x = 1.0;
	m_orientation.y = 1.0;
}

APcopter_arucoLanding::~APcopter_arucoLanding()
{
}

bool APcopter_arucoLanding::init(void* pKiss)
{
	IF_F(this->ActionBase::init(pKiss) == false);
	Kiss* pK = (Kiss*) pKiss;

	pK->v("orientationX", &m_orientation.x);
	pK->v("orientationY", &m_orientation.y);

	m_oTarget.init();

	Kiss** pItrT = pK->getChildItr();
	LANDING_TARGET_ARUCO L;
	int i = 0;
	while (pItrT[i])
	{
		Kiss* pKs = pItrT[i++];
		L.init();
		F_ERROR_F(pKs->v("tag", &L.m_tag));
		pKs->v("angle", &L.m_angle);
		m_vTarget.push_back(L);
	}

	//link
	string iName = "";
	F_INFO(pK->v("APcopter_base", &iName));
	m_pAP = (APcopter_base*) (pK->parent()->getChildInst(iName));

	F_ERROR_F(pK->v("_ArUco", &iName));
	m_pArUco = (_ObjectBase*) (pK->root()->getChildInst(iName));

	F_INFO(pK->v("_DepthVisionBase", &iName));
	m_pDV = (_DepthVisionBase*) (pK->root()->getChildInst(iName));

	return true;
}

void APcopter_arucoLanding::update(void)
{
	this->ActionBase::update();

	NULL_(m_pAP);
	NULL_(m_pAP->m_pMavlink);
	NULL_(m_pArUco);
	_VisionBase* pV = m_pArUco->m_pVision;
	NULL_(pV);

	IF_(!isActive());

	vInt2 cSize;
	vInt2 cCenter;
	vDouble2 cAngle;
	pV->info(&cSize, &cCenter, &cAngle);

	int iDet = 0;
	int iPriority = INT_MAX;
	LANDING_TARGET_ARUCO* pTarget = NULL;

	while(1)
	{
		OBJECT* pO = m_pArUco->at(iDet++);
		if(!pO)break;

		LANDING_TARGET_ARUCO* pT = NULL;
		int i;
		for(i=0; i<m_vTarget.size(); i++)
		{
			LANDING_TARGET_ARUCO* pTi = &m_vTarget[i];
			IF_CONT(pO->m_topClass != pTi->m_tag);

			pT = pTi;
			break;
		}

		IF_CONT(!pT);
		IF_CONT(i >= iPriority);

		pTarget = pT;
		iPriority = i;
		m_oTarget = *pO;
	}

	if(!pTarget)
	{
		m_bLocked = false;
		return;
	}
	m_bLocked = true;

	m_D.target_num = 0;
	m_D.frame = MAV_FRAME_BODY_NED;
	m_D.distance = 0;
	m_D.size_x = 0;
	m_D.size_y = 0;

	//Change position to angles
	m_D.angle_x = (float)((double)(m_oTarget.m_bb.x - cCenter.x) / (double)cSize.x)
							* cAngle.x * DEG_RAD * m_orientation.x;
	m_D.angle_y = (float)((double)(m_oTarget.m_bb.y - cCenter.y) / (double)cSize.y)
							* cAngle.y * DEG_RAD * m_orientation.y;

	//Use depth if available
	if(m_pDV)
	{
//		m_D.distance = (float)m_pDV->dMedian();
		if(m_D.distance < 0.0)m_D.distance = 0.0;
	}

	m_pAP->m_pMavlink->landingTarget(m_D);
}

bool APcopter_arucoLanding::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	string msg = *this->getName() + ": ";

	if(!isActive())
	{
		msg += "Inactive";
	}
	else if (m_bLocked)
	{
		circle(*pMat, Point(m_oTarget.m_bb.x, m_oTarget.m_bb.y),
				pMat->cols * pMat->rows * 0.0001, Scalar(0, 0, 255), 2);

		msg += "Target tag = " + i2str(m_oTarget.m_topClass)
				+ ", angle = ("
				+ f2str((double)m_D.angle_x) + " , "
				+ f2str((double)m_D.angle_y) + ")"
				+ ", d=" + f2str(m_D.distance);
	}
	else
	{
		msg += "Target tag not found";
	}

	pWin->addMsg(&msg);

	return true;
}

bool APcopter_arucoLanding::cli(int& iY)
{
	IF_F(!this->ActionBase::cli(iY));
	NULL_F(m_pAP->m_pMavlink);

	string msg;

	if(!isActive())
	{
		msg = "Inactive";
	}
	else if (m_bLocked)
	{
		msg = "Target tag = " + i2str(m_oTarget.m_topClass)
				+ ", angle = ("
				+ f2str((double)m_D.angle_x) + " , "
				+ f2str((double)m_D.angle_y) + ")"
				+ ", d=" + f2str(m_D.distance);
	}
	else
	{
		msg = "Target tag not found";
	}

	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}

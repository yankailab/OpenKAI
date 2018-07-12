#include "APcopter_arucoLanding.h"

namespace kai
{

APcopter_arucoLanding::APcopter_arucoLanding()
{
	m_pAP = NULL;
	m_pArUco = NULL;

	m_tAngle.init();
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
	pK->m_pInst = this;

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

	return true;
}

bool APcopter_arucoLanding::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("APcopter_base", &iName));
	m_pAP = (APcopter_base*) (pK->parent()->getChildInstByName(&iName));

	F_ERROR_F(pK->v("_ArUco", &iName));
	m_pArUco = (_ObjectBase*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void APcopter_arucoLanding::update(void)
{
	this->ActionBase::update();

	navigation();
}

void APcopter_arucoLanding::navigation(void)
{
	NULL_(m_pAP);
	NULL_(m_pAP->m_pMavlink);
	NULL_(m_pArUco);
	_VisionBase* pV = m_pArUco->m_pVision;
	NULL_(pV);

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
			IF_CONT(pO->m_iClass != pTi->m_tag);

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

	//Change position to angles
	m_tAngle.x = ((double)(m_oTarget.m_fBBox.x - cCenter.x) / (double)cSize.x)
				* cAngle.x * DEG_RAD * m_orientation.x;
	m_tAngle.y = ((double)(m_oTarget.m_fBBox.y - cCenter.y) / (double)cSize.y)
				* cAngle.y * DEG_RAD * m_orientation.y;

	//Send Mavlink command
	mavlink_landing_target_t D;
	D.target_num = 0;
	D.frame = MAV_FRAME_BODY_NED;
	D.angle_x = m_tAngle.x;
	D.angle_y = m_tAngle.y;
	D.distance = 0;
	D.size_x = 0;
	D.size_y = 0;

	m_pAP->m_pMavlink->landingTarget(D);
}

bool APcopter_arucoLanding::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	string msg;

	if (m_bLocked)
	{
		circle(*pMat, Point(m_oTarget.m_fBBox.x, m_oTarget.m_fBBox.y),
				pMat->cols * pMat->rows * 0.0001, Scalar(0, 0, 255), 2);

		msg = "Target tag = " + i2str(m_oTarget.m_iClass)
				+ ", angle = ("
				+ f2str(m_tAngle.x) + " , "
				+ f2str(m_tAngle.y) + ")";
	}
	else
	{
		msg = "Target tag not found";
	}

	pWin->addMsg(&msg);

	return true;
}

bool APcopter_arucoLanding::cli(int& iY)
{
	IF_F(!this->ActionBase::cli(iY));
	NULL_F(m_pAP->m_pMavlink);

	string msg;

	if (m_bLocked)
	{
		msg = "Target tag = " + i2str(m_oTarget.m_iClass)
				+ ", angle = ("
				+ f2str(m_tAngle.x) + " , "
				+ f2str(m_tAngle.y) + ")";
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

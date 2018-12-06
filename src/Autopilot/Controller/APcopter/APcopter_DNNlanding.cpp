#include "APcopter_DNNlanding.h"

namespace kai
{

APcopter_DNNlanding::APcopter_DNNlanding()
{
	m_pAP = NULL;
	m_pAI = NULL;
	m_target.init();
}

APcopter_DNNlanding::~APcopter_DNNlanding()
{
}

bool APcopter_DNNlanding::init(void* pKiss)
{
	IF_F(this->ActionBase::init(pKiss) == false);
	Kiss* pK = (Kiss*) pKiss;

	F_INFO(pK->v("orientationX", &m_target.m_orientX));
	F_INFO(pK->v("orientationY", &m_target.m_orientY));
	F_INFO(pK->v("timeout", &m_target.m_timeOut));

	//link
	string iName = "";
	F_INFO(pK->v("APcopter_base", &iName));
	m_pAP = (APcopter_base*) (pK->parent()->getChildInst(iName));

	F_ERROR_F(pK->v("_AIbase", &iName));
	m_pAI = (_ObjectBase*) (pK->root()->getChildInst(iName));
	return true;
}

void APcopter_DNNlanding::update(void)
{
	this->ActionBase::update();

	NULL_(m_pAP);

	landing();
}

void APcopter_DNNlanding::landing(void)
{
	NULL_(m_pAP);
	NULL_(m_pAI);
	_VisionBase* pVision = m_pAI->m_pVision;
	NULL_(pVision);

	vInt2 cSize;
	vInt2 cCenter;
	vDouble2 cAngle;
	pVision->info(&cSize, &cCenter, &cAngle);

	vInt4 targetBB;
	targetBB.init();

	OBJECT* pO = m_pAI->at(0);
	if (!pO)
	{
		m_target.m_targetPos.init();
		m_target.m_bLocked = false;
		return;
	}

	targetBB = pO->iBBox(cSize);
	m_target.m_targetPos.x = targetBB.midX();
	m_target.m_targetPos.y = targetBB.midY();
	m_target.m_bLocked = true;

	//Change position to angles
	m_target.m_angleX = ((double)(m_target.m_targetPos.x - cCenter.x) / (double)cSize.x)
			* cAngle.x * DEG_RAD * m_target.m_orientX;
	m_target.m_angleY = ((double)(m_target.m_targetPos.y - cCenter.y) / (double)cSize.y)
			* cAngle.y * DEG_RAD * m_target.m_orientY;

	//Send Mavlink command
	mavlink_landing_target_t D;
	D.target_num = 0;
	D.frame = MAV_FRAME_BODY_NED;
	D.angle_x = m_target.m_angleX;
	D.angle_y = m_target.m_angleY;
	D.distance = 0;
	D.size_x = 0;
	D.size_y = 0;

	m_pAP->m_pMavlink->landingTarget(D);
}

bool APcopter_DNNlanding::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	string msg;

	if (m_target.m_bLocked)
	{
		circle(*pMat, Point(m_target.m_targetPos.x, m_target.m_targetPos.y),
				pMat->cols * pMat->rows * 0.0001, Scalar(0, 255, 0), 2);

		msg = "Landing_Target: (" + f2str(m_target.m_angleX) + " , "
				+ f2str(m_target.m_angleY) + ")";
	}
	else
	{
		msg = "Landing_Target: Target not found";
	}

	pWin->addMsg(msg);

	return true;
}

}


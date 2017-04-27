#include "../APcopter/APcopter_landing.h"

namespace kai
{

APcopter_landing::APcopter_landing()
{
	m_pAP = NULL;
	m_pAI = NULL;
	m_target.init();
}

APcopter_landing::~APcopter_landing()
{
}

bool APcopter_landing::init(void* pKiss)
{
	IF_F(this->ActionBase::init(pKiss) == false);
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("orientationX", &m_target.m_orientX));
	F_INFO(pK->v("orientationY", &m_target.m_orientY));
	F_INFO(pK->v("timeout", &m_target.m_timeOut));

	return true;
}

bool APcopter_landing::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("APcopter_base", &iName));
	m_pAP = (APcopter_base*) (pK->parent()->getChildInstByName(&iName));

	F_ERROR_F(pK->v("_AIbase", &iName));
	m_pAI = (_AIbase*) (pK->root()->getChildInstByName(&iName));
	return true;
}

void APcopter_landing::update(void)
{
	this->ActionBase::update();

	NULL_(m_pAP);

	landing();
}

void APcopter_landing::landing(void)
{
	NULL_(m_pAP);
	NULL_(m_pAI);
	_VisionBase* pStream = m_pAI->m_pStream;
	NULL_(pStream);

	vInt2 cSize;
	vInt2 cCenter;
	vInt2 cAngle;
	pStream->info(&cSize, &cCenter, &cAngle);

	vInt4 targetBB;
	targetBB.init();
	uint64_t frameID = get_time_usec() - m_target.m_timeOut;

	int k = 0;
	for (int i = 0; i < m_pAI->size(); i++)
	{
		OBJECT* pObj = m_pAI->get(i, frameID);
		if (!pObj)
			continue;

		targetBB = pObj->m_bbox;
		m_target.m_targetPos.x = pObj->m_bbox.midX();
		m_target.m_targetPos.y = pObj->m_bbox.midY();
		k++;
		break;
	}

	if (k <= 0)
	{
		m_target.m_targetPos.init();
		m_target.m_bLocked = false;
		return;
	}

	m_target.m_bLocked = true;

	//Change position to angles
	m_target.m_angleX = ((double)(m_target.m_targetPos.x - cCenter.x) / (double)cSize.x)
			* cAngle.x * DEG_RAD * m_target.m_orientX;
	m_target.m_angleY = ((double)(m_target.m_targetPos.y - cCenter.y) / (double)cSize.y)
			* cAngle.y * DEG_RAD * m_target.m_orientY;

	//Send Mavlink command
	m_pAP->m_pMavlink->landingTarget(MAV_DATA_STREAM_ALL, MAV_FRAME_BODY_NED,
			m_target.m_angleX, m_target.m_angleY, 0, 0, 0);

}

bool APcopter_landing::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
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

	pWin->addMsg(&msg);

	return true;
}

}


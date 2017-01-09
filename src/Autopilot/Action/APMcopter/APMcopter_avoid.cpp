#include "APMcopter_avoid.h"

namespace kai
{

APMcopter_avoid::APMcopter_avoid()
{
	ActionBase();

	m_pSF40 = NULL;
	m_pAPM = NULL;
	m_pObs = NULL;
	m_avoidArea.m_x = 0.0;
	m_avoidArea.m_y = 0.0;
	m_avoidArea.m_z = 1.0;
	m_avoidArea.m_w = 1.0;
}

APMcopter_avoid::~APMcopter_avoid()
{
}

bool APMcopter_avoid::init(void* pKiss)
{
	CHECK_F(this->ActionBase::init(pKiss) == false);
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("avoidLeft", &m_avoidArea.m_x));
	F_INFO(pK->v("avoidTop", &m_avoidArea.m_y));
	F_INFO(pK->v("avoidRight", &m_avoidArea.m_z));
	F_INFO(pK->v("avoidBottom", &m_avoidArea.m_w));

	return true;
}

bool APMcopter_avoid::link(void)
{
	CHECK_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";

	F_INFO(pK->v("APMcopter_base", &iName));
	m_pAPM = (APMcopter_base*) (pK->parent()->getChildInstByName(&iName));

	F_INFO(pK->v("_Lightware_SF40", &iName));
	m_pSF40 = (_Lightware_SF40*) (pK->root()->getChildInstByName(&iName));

	F_INFO(pK->v("_Obstacle", &iName));
	m_pObs = (_Obstacle*) (pK->root()->getChildInstByName(&iName));

	//Add GPS

	return true;
}

void APMcopter_avoid::update(void)
{
	this->ActionBase::update();

	NULL_(m_pAPM);

	updateDistanceSensor();
}

void APMcopter_avoid::updateDistanceSensor(void)
{
	NULL_(m_pAPM->m_pMavlink);
	NULL_(m_pObs);

	double rangeMin, rangeMax;
	uint8_t orientation;
	m_pObs->info(&rangeMin, &rangeMax, &orientation);
	m_DS.m_distance = rangeMax;

	OBSTACLE* pPrimary = NULL;
	if (m_pObs)
	{
		int64_t frameID = get_time_usec() - m_pObs->m_dTime;

		//get the closest object
		for (int i = 0; i < m_pObs->size(); i++)
		{
			OBSTACLE* pO = m_pObs->get(i, frameID);
			if (!pO)
				continue;
			if (pO->m_dist < 0.0)
				continue;

			//check if inside avoid area
			vInt4* pBB = &pO->m_bbox;
			vInt2* pCam = &pO->m_camSize;
			vInt4 avoidR;
			avoidR.m_x = pCam->m_x * m_avoidArea.m_x;
			avoidR.m_y = pCam->m_y * m_avoidArea.m_y;
			avoidR.m_z = pCam->m_x * m_avoidArea.m_z;
			avoidR.m_w = pCam->m_y * m_avoidArea.m_w;

			if(!isOverlapped(pBB,&avoidR))
				continue;

			if (pO->m_dist < m_DS.m_distance)
			{
				m_DS.m_distance = pO->m_dist;
				pPrimary = pO;
			}
		}
	}

	if(pPrimary)
		pPrimary->m_bPrimaryTarget = true;

	m_DS.m_maxDistance = rangeMax * 0.1;
	m_DS.m_minDistance = rangeMin * 0.1;
	m_DS.m_orientation = orientation;
	m_DS.m_type = 0;
	m_pAPM->updateDistanceSensor(&m_DS);
}

bool APMcopter_avoid::draw(void)
{
	CHECK_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	string msg = *this->getName() + " Dist=" + i2str(m_DS.m_distance);
	pWin->addMsg(&msg);

	Mat* pMat = pWin->getFrame()->getCMat();
	Rect r;
	r.x = m_avoidArea.m_x * ((double)pMat->cols);
	r.y = m_avoidArea.m_y * ((double)pMat->rows);
	r.width = m_avoidArea.m_z * ((double)pMat->cols) - r.x;
	r.height = m_avoidArea.m_w * ((double)pMat->rows) - r.y;

	rectangle(*pMat, r, Scalar(0,255,255), 1);
	return true;
}

}

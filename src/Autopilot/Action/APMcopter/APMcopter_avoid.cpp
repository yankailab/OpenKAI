#include "APMcopter_avoid.h"

namespace kai
{

APMcopter_avoid::APMcopter_avoid()
{
	m_pSF40 = NULL;
	m_pAPM = NULL;
	m_pObs = NULL;
	m_pZED = NULL;
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

	F_INFO(pK->v("_ZED", &iName));
	m_pZED = (_ZED*) (pK->root()->getChildInstByName(&iName));

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
	NULL_(m_pZED);

	double rangeMin, rangeMax;
	m_pZED->getRange(&rangeMin, &rangeMax);

	m_DS.m_distance = m_pObs->dist(&m_avoidArea,&m_posMin) * 100;
	m_DS.m_maxDistance = rangeMax * 100;
	m_DS.m_minDistance = rangeMin * 100;
	m_DS.m_orientation = 0;
	m_DS.m_type = 0;

	if(m_DS.m_distance < m_DS.m_minDistance || m_DS.m_distance > m_DS.m_maxDistance)
	{
		m_DS.m_distance = m_DS.m_maxDistance;
	}

	m_pAPM->updateDistanceSensor(&m_DS);
}

bool APMcopter_avoid::draw(void)
{
	CHECK_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	CHECK_F(pMat->empty());

	string msg = *this->getName() + " Dist=" + i2str(m_DS.m_distance);
	pWin->addMsg(&msg);

	Rect r;
	r.x = m_avoidArea.m_x * ((double)pMat->cols);
	r.y = m_avoidArea.m_y * ((double)pMat->rows);
	r.width = m_avoidArea.m_z * ((double)pMat->cols) - r.x;
	r.height = m_avoidArea.m_w * ((double)pMat->rows) - r.y;
	rectangle(*pMat, r, Scalar(0,255,255), 1);

	NULL_F(m_pObs);
	vInt2 mDim = m_pObs->matrixDim();
	circle(*pMat, Point((m_posMin.m_x+0.5)*(pMat->cols/mDim.m_x), (m_posMin.m_y+0.5)*(pMat->rows/mDim.m_y)),
			0.000025*pMat->cols*pMat->rows,
			Scalar(0, 255, 255), 2);

	return true;
}

}

#include "APMcopter_avoid.h"

namespace kai
{

APMcopter_avoid::APMcopter_avoid()
{
	m_pSF40 = NULL;
	m_pAPM = NULL;
	m_pObs = NULL;
	m_pZED = NULL;

	m_distObs = 0.0;
	m_distSF40 = 0.0;

	m_avoidArea.x = 0.0;
	m_avoidArea.y = 0.0;
	m_avoidArea.z = 1.0;
	m_avoidArea.w = 1.0;
}

APMcopter_avoid::~APMcopter_avoid()
{
}

bool APMcopter_avoid::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("avoidLeft", &m_avoidArea.x));
	F_INFO(pK->v("avoidTop", &m_avoidArea.y));
	F_INFO(pK->v("avoidRight", &m_avoidArea.z));
	F_INFO(pK->v("avoidBottom", &m_avoidArea.w));

	return true;
}

bool APMcopter_avoid::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;
	string iName;

	iName = "";
	F_INFO(pK->v("APMcopter_base", &iName));
	m_pAPM = (APMcopter_base*) (pK->parent()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_Lightware_SF40", &iName));
	m_pSF40 = (_Lightware_SF40*) (pK->root()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_Obstacle", &iName));
	m_pObs = (_Obstacle*) (pK->root()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_ZED", &iName));
	m_pZED = (_ZED*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void APMcopter_avoid::update(void)
{
	this->ActionBase::update();

	updateZED();
	updateSF40();
}

void APMcopter_avoid::updateZED(void)
{
	IF_(!m_pZED);
	IF_(!m_pObs);
	IF_(!m_pZED->isOpened());
	NULL_(m_pAPM);
	NULL_(m_pAPM->m_pMavlink);
	_Mavlink* pMavlink = m_pAPM->m_pMavlink;

	int i;
	double maxDist;
	double minDist;

	double rangeMin, rangeMax;
	m_pZED->getRange(&rangeMin, &rangeMax);

	m_distObs = m_pObs->dist(&m_avoidArea,&m_posMin) * 100;
	maxDist = rangeMax * 100;
	minDist = rangeMin * 100;

	if(m_distObs < minDist || m_distObs > maxDist)
		m_distObs = maxDist;

	pMavlink->distance_sensor(0, //type
			0,	//orientation
			maxDist,
			minDist,
			m_distObs);
}

void APMcopter_avoid::updateSF40(void)
{
	IF_(!m_pSF40);
	NULL_(m_pAPM);
	NULL_(m_pAPM->m_pMavlink);
	_Mavlink* pMavlink = m_pAPM->m_pMavlink;

	int i;
	double maxDist;
	double minDist;

	maxDist = m_pSF40->maxDist();
	minDist = m_pSF40->minDist();

	//send 8 orientations
	const double pAngles[] = {0, 45, 90, 135, 180, 225, 270, 315};
	const int nAngles = 8;

	for(i=0; i<nAngles; i++)
	{
		m_distSF40 = m_pSF40->getDistance(pAngles[i]) * 100;

		if(m_distSF40 < minDist || m_distSF40 > maxDist)
			m_distSF40 = maxDist;

		pMavlink->distance_sensor(0, //type
				i,	//orientation
				maxDist,
				minDist,
				m_distSF40);
	}
}

bool APMcopter_avoid::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	IF_F(pMat->empty());

	string msg;

	if(m_pZED && m_pObs)
	{
		msg = *this->getName() + " Obs dist=" + i2str(m_distObs);
		pWin->addMsg(&msg);
	}

	if(m_pSF40)
	{
		msg = *this->getName() + " SF40 dist=" + i2str(m_distSF40);
		pWin->addMsg(&msg);
	}

	Rect r;
	r.x = m_avoidArea.x * ((double)pMat->cols);
	r.y = m_avoidArea.y * ((double)pMat->rows);
	r.width = m_avoidArea.z * ((double)pMat->cols) - r.x;
	r.height = m_avoidArea.w * ((double)pMat->rows) - r.y;
	rectangle(*pMat, r, Scalar(0,255,255), 1);

	NULL_F(m_pObs);
	vInt2 mDim = m_pObs->matrixDim();
	circle(*pMat, Point((m_posMin.x+0.5)*(pMat->cols/mDim.x), (m_posMin.y+0.5)*(pMat->rows/mDim.y)),
			0.000025*pMat->cols*pMat->rows,
			Scalar(0, 255, 255), 2);

	return true;
}

}

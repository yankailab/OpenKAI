#include "APcopter_distLidar.h"

namespace kai
{

APcopter_distLidar::APcopter_distLidar()
{
	m_pAP = NULL;
	m_pDS = NULL;
	m_nSection = 0;
}

APcopter_distLidar::~APcopter_distLidar()
{
}

bool APcopter_distLidar::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	return true;
}

bool APcopter_distLidar::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;
	string iName;

	iName = "";
	F_INFO(pK->v("APcopter_base", &iName));
	m_pAP = (APcopter_base*) (pK->parent()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_DistSensorBase", &iName));
	m_pDS = (_DistSensorBase*) (pK->root()->getChildInstByName(&iName));
	IF_Fl(!m_pDS,iName << " not found");

	Kiss** pItrDS = pK->getChildItr();
	m_nSection = 0;

	while (pItrDS[m_nSection])
	{
		Kiss* pKs = pItrDS[m_nSection];
		IF_F(m_nSection >= N_LIDAR_SECTION);

		DIST_LIDAR_SECTION* pS = &m_pSection[m_nSection];
		pS->init();
		F_ERROR_F(pKs->v("orientation", (int*)&pS->m_orientation));
		F_ERROR_F(pKs->v("degFrom", &pS->m_degFrom));
		F_ERROR_F(pKs->v("degTo", &pS->m_degTo));

		IF_Fl(pS->m_degFrom < 0, "degFrom < 0 deg");
		IF_Fl(pS->m_degTo < 0, "degTo < 0 deg");
		IF_Fl(pS->m_degTo <= pS->m_degFrom, "Angle width <= 0 deg");
		IF_Fl(pS->m_degTo - pS->m_degFrom > 180, "Angle width > 180 deg");

		m_nSection++;
	}

	return true;
}

void APcopter_distLidar::update(void)
{
	this->ActionBase::update();

	updateMavlink();
}

void APcopter_distLidar::updateMavlink(void)
{
	NULL_(m_pAP);
	NULL_(m_pAP->m_pMavlink);
	_Mavlink* pMavlink = m_pAP->m_pMavlink;

	NULL_(m_pDS);
	IF_(!m_pDS->bReady());

	double rMin = m_pDS->rMin();
	double rMax = m_pDS->rMax();

	for(int i=0; i<m_nSection; i++)
	{
		DIST_LIDAR_SECTION* pS = &m_pSection[i];

		double d = m_pDS->dMin(pS->m_degFrom, pS->m_degTo);
		if(d < rMin)d = rMax;
		pS->m_minD = d;

		pMavlink->distanceSensor(
				0,
				pS->m_orientation,
				(uint16_t)(rMax*100),
				(uint16_t)(rMin*100),
				(uint16_t)(pS->m_minD * 100));

		LOG_I("orient: " << i2str(pS->m_orientation) << " minD: " << f2str(pS->m_minD));
	}

}

bool APcopter_distLidar::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	Point pCenter(pMat->cols / 2, pMat->rows / 2);
	Scalar col = Scalar(0, 255, 0);
	double rMax = m_pDS->rMax() * m_pDS->m_showScale;

	for(int i=0; i<m_nSection; i++)
	{
		DIST_LIDAR_SECTION* pS = &m_pSection[i];

		double radFrom = pS->m_degFrom * DEG_RAD;
		double radTo = pS->m_degTo * DEG_RAD;
		double d = pS->m_minD * m_pDS->m_showScale / cos(0.5 * (radFrom + radTo) - radFrom);

		vDouble2 pFrom,pTo;
		pFrom.x = sin(radFrom);
		pFrom.y = -cos(radFrom);
		pTo.x = sin(radTo);
		pTo.y = -cos(radTo);

		line(*pMat, pCenter + Point(pFrom.x*d,pFrom.y*d), pCenter + Point(pTo.x*d,pTo.y*d), col, 2);
		line(*pMat, pCenter + Point(pFrom.x*rMax,pFrom.y*rMax), pCenter, col, 1);
		line(*pMat, pCenter, pCenter + Point(pTo.x*rMax,pTo.y*rMax), col, 1);
	}

	return true;
}

}


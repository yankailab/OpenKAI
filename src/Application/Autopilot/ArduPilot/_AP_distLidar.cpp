#include "../ArduPilot/_AP_distLidar.h"

namespace kai
{

_AP_distLidar::_AP_distLidar()
{
	m_pAP = NULL;
	m_pDS = NULL;
	m_nSection = 0;
}

_AP_distLidar::~_AP_distLidar()
{
}

bool _AP_distLidar::init(void* pKiss)
{
	IF_F(!this->_StateBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	string n;

	n = "";
	F_INFO(pK->v("APcopter_base", &n));
	m_pAP = (_AP_base*) (pK->getInst(n));

	n = "";
	F_INFO(pK->v("_DistSensorBase", &n));
	m_pDS = (_DistSensorBase*) (pK->getInst(n));
	IF_Fl(!m_pDS,n + " not found");

	m_nSection = 0;
	while (1)
	{
		IF_F(m_nSection >= N_LIDAR_SECTION);
		Kiss* pKs = pK->child(m_nSection);
		if(pKs->empty())break;

		DIST_LIDAR_SECTION* pS = &m_pSection[m_nSection];
		pS->init();
		F_ERROR_F(pKs->v("orientation", (int*)&pS->m_orientation));
		F_ERROR_F(pKs->v("degFrom", &pS->m_degFrom));
		F_ERROR_F(pKs->v("degTo", &pS->m_degTo));
		F_INFO(pKs->v("sensorScale", &pS->m_sensorScale));

		IF_Fl(pS->m_degFrom < 0, "degFrom < 0 deg");
		IF_Fl(pS->m_degTo < 0, "degTo < 0 deg");
		IF_Fl(pS->m_degTo <= pS->m_degFrom, "Angle width <= 0 deg");
		IF_Fl(pS->m_degTo - pS->m_degFrom > 180, "Angle width > 180 deg");

		m_nSection++;
	}

	return true;
}

void _AP_distLidar::update(void)
{
	this->_StateBase::update();

	updateMavlink();
}

void _AP_distLidar::updateMavlink(void)
{
	NULL_(m_pAP);
	NULL_(m_pAP->m_pMav);
	_Mavlink* pMavlink = m_pAP->m_pMav;

	NULL_(m_pDS);
	IF_(!m_pDS->bReady());

	double rMin = m_pDS->rMin();
	double rMax = m_pDS->rMax();

	mavlink_distance_sensor_t D;

	for(int i=0; i<m_nSection; i++)
	{
		DIST_LIDAR_SECTION* pS = &m_pSection[i];

		double d = m_pDS->dMin(pS->m_degFrom, pS->m_degTo) * pS->m_sensorScale;
		if(d < rMin)d = rMax;
		if(d > rMax)d = rMax;
		pS->m_minD = d;

		D.type = 0;
		D.max_distance = (uint16_t)(rMax*100);	//unit: centimeters
		D.min_distance = (uint16_t)(rMin*100);
		D.current_distance = (uint16_t)(pS->m_minD * 100);
		D.orientation = pS->m_orientation;
		D.covariance = 255;

		pMavlink->distanceSensor(D);
		LOG_I("orient: " + i2str(pS->m_orientation) + " minD: " + f2str(pS->m_minD));
	}
}

void _AP_distLidar::draw(void)
{
	this->_StateBase::draw();

#ifdef USE_OPENCV
	IF_(!checkWindow());

	_WindowCV* pWin = (_WindowCV*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	Point pCenter(pMat->cols / 2, pMat->rows / 2);
	Scalar col = Scalar(0, 255, 0);
	double rMax = m_pDS->rMax() * m_pDS->m_showScale;

	for(int i=0; i<m_nSection; i++)
	{
		DIST_LIDAR_SECTION* pS = &m_pSection[i];

		double radFrom = pS->m_degFrom * DEG_2_RAD;
		double radTo = pS->m_degTo * DEG_2_RAD;
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
#endif
}

}


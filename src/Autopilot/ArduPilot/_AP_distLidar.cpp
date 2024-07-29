#include "../ArduPilot/_AP_distLidar.h"

namespace kai
{

	_AP_distLidar::_AP_distLidar()
	{
		m_pAP = nullptr;
		m_pDS = nullptr;
		m_nSection = 0;
	}

	_AP_distLidar::~_AP_distLidar()
	{
	}

	int _AP_distLidar::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;
    	
		string n;

		n = "";
		F_INFO(pK->v("APcopter_base", &n));
		m_pAP = (_AP_base *)(pK->findModule(n));

		n = "";
		F_INFO(pK->v("_DistSensorBase", &n));
		m_pDS = (_DistSensorBase *)(pK->findModule(n));
		NULL__(m_pDS, OK_ERR_NOT_FOUND);

		m_nSection = 0;
		while (1)
		{
			IF_F(m_nSection >= N_LIDAR_SECTION);
			Kiss *pKs = pK->child(m_nSection);
			if (pKs->empty())
				break;

			DIST_LIDAR_SECTION *pS = &m_pSection[m_nSection];
			pS->init();
			pKs->v("orientation", (int *)&pS->m_orientation);
			pKs->v("degFrom", &pS->m_degFrom);
			pKs->v("degTo", &pS->m_degTo);
			pKs->v("sensorScale", &pS->m_sensorScale);

			IF__(pS->m_degFrom < 0, OK_ERR_INVALID_VALUE);
			IF__(pS->m_degTo < 0, OK_ERR_INVALID_VALUE);
			IF__(pS->m_degTo <= pS->m_degFrom, OK_ERR_INVALID_VALUE);
			IF__(pS->m_degTo - pS->m_degFrom > 180, OK_ERR_INVALID_VALUE);

			m_nSection++;
		}

		return OK_OK;
	}

	void _AP_distLidar::update(void)
	{
		updateMavlink();
	}

	void _AP_distLidar::updateMavlink(void)
	{
		NULL_(m_pAP);
		NULL_(m_pAP->m_pMav);
		_Mavlink *pMavlink = m_pAP->m_pMav;

		NULL_(m_pDS);
		IF_(!m_pDS->bReady());

		double rMin = m_pDS->rMin();
		double rMax = m_pDS->rMax();

		mavlink_distance_sensor_t D;

		for (int i = 0; i < m_nSection; i++)
		{
			DIST_LIDAR_SECTION *pS = &m_pSection[i];

			double d = m_pDS->dMin(pS->m_degFrom, pS->m_degTo) * pS->m_sensorScale;
			if (d < rMin)
				d = rMax;
			if (d > rMax)
				d = rMax;
			pS->m_minD = d;

			D.type = 0;
			D.max_distance = (uint16_t)(rMax * 100); //unit: centimeters
			D.min_distance = (uint16_t)(rMin * 100);
			D.current_distance = (uint16_t)(pS->m_minD * 100);
			D.orientation = pS->m_orientation;
			D.covariance = 255;

			pMavlink->distanceSensor(D);
			LOG_I("orient: " + i2str(pS->m_orientation) + " minD: " + f2str(pS->m_minD));
		}
	}

	void _AP_distLidar::draw(void *pFrame)
	{
#ifdef USE_OPENCV
		NULL_(pFrame);
		this->_ModuleBase::draw(pFrame);
		IF_(check() != OK_OK);

		Frame *pF = (Frame *)pFrame;

		Mat *pM = pF->m();
		IF_(pM->empty());

		// Point pCenter(pM->cols / 2, pM->rows / 2);
		// Scalar col = Scalar(0, 255, 0);
		// double rMax = m_pDS->rMax() * m_pDS->m_showScale;

		// for (int i = 0; i < m_nSection; i++)
		// {
		// 	DIST_LIDAR_SECTION *pS = &m_pSection[i];

		// 	double radFrom = pS->m_degFrom * DEG_2_RAD;
		// 	double radTo = pS->m_degTo * DEG_2_RAD;
		// 	double d = pS->m_minD * m_pDS->m_showScale / cos(0.5 * (radFrom + radTo) - radFrom);

		// 	vDouble2 pFrom, pTo;
		// 	pFrom.x = sin(radFrom);
		// 	pFrom.y = -cos(radFrom);
		// 	pTo.x = sin(radTo);
		// 	pTo.y = -cos(radTo);

		// 	line(*pM, pCenter + Point(pFrom.x * d, pFrom.y * d), pCenter + Point(pTo.x * d, pTo.y * d), col, 2);
		// 	line(*pM, pCenter + Point(pFrom.x * rMax, pFrom.y * rMax), pCenter, col, 1);
		// 	line(*pM, pCenter, pCenter + Point(pTo.x * rMax, pTo.y * rMax), col, 1);
		// }
#endif
	}

}

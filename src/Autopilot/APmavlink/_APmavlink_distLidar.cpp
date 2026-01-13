#include "_APmavlink_distLidar.h"

namespace kai
{

	_APmavlink_distLidar::_APmavlink_distLidar()
	{
		m_pAP = nullptr;
		m_pDS = nullptr;
		m_nSection = 0;
	}

	_APmavlink_distLidar::~_APmavlink_distLidar()
	{
	}

	bool _APmavlink_distLidar::init(const json &j)
	{
		IF_F(!this->_ModuleBase::init(j));

		const json &js = j.at("sections");
		IF__(!js.is_object(), true);

		m_nSection = 0;
		for (auto it = js.begin(); it != js.end(); it++)
		{
			const json &ji = it.value();
			IF_CONT(!ji.is_object());

			DIST_LIDAR_SECTION *pS = &m_pSection[m_nSection];
			pS->init();
			pS->m_orientation = ji.value("orientation", pS->m_orientation);
			pS->m_degFrom = ji.value("degFrom", pS->m_degFrom);
			pS->m_degTo = ji.value("degTo", pS->m_degTo);
			pS->m_sensorScale = ji.value("sensorScale", pS->m_sensorScale);

			IF_F(pS->m_degFrom < 0);
			IF_F(pS->m_degTo < 0);
			IF_F(pS->m_degTo <= pS->m_degFrom);
			IF_F(pS->m_degTo - pS->m_degFrom > 180);

			m_nSection++;
		}

		return true;
	}

	bool _APmavlink_distLidar::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		string n;

		n = j.value("APmavlink_base", "");
		m_pAP = (_APmavlink_base *)(pM->findModule(n));
		NULL_F(m_pAP);

		n = j.value("_DistSensorBase", "");
		m_pDS = (_DistSensorBase *)(pM->findModule(n));
		NULL_F(m_pDS);

		return true;
	}

	void _APmavlink_distLidar::update(void)
	{
		updateMavlink();
	}

	void _APmavlink_distLidar::updateMavlink(void)
	{
		NULL_(m_pAP);
		NULL_(m_pAP->getMavlink());
		_Mavlink *pMavlink = m_pAP->getMavlink();

		NULL_(m_pDS);
		IF_(!m_pDS->bReady());

		vFloat2 vRange = m_pDS->range();

		mavlink_distance_sensor_t D;
		for (int i = 0; i < m_nSection; i++)
		{
			DIST_LIDAR_SECTION *pS = &m_pSection[i];

			float d = m_pDS->dMin(pS->m_degFrom, pS->m_degTo) * pS->m_sensorScale;
			d = vRange.constrain(d);
			pS->m_minD = d;

			D.type = 0;
			D.max_distance = (uint16_t)(vRange.y * 100); // unit: centimeters
			D.min_distance = (uint16_t)(vRange.x * 100);
			D.current_distance = (uint16_t)(pS->m_minD * 100);
			D.orientation = pS->m_orientation;
			D.covariance = 255;

			pMavlink->distanceSensor(D);
			LOG_I("orient: " + i2str(pS->m_orientation) + " minD: " + f2str(pS->m_minD));
		}
	}

	void _APmavlink_distLidar::draw(void *pFrame)
	{
#ifdef USE_OPENCV
		NULL_(pFrame);
		this->_ModuleBase::draw(pFrame);
		IF_(!check());

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

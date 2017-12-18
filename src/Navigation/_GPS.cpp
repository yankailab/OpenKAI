/*
 * _GPS.cpp
 *
 *  Created on: Jan 6, 2017
 *      Author: yankai
 */

#include "_GPS.h"

namespace kai
{

_GPS::_GPS()
{
#ifdef USE_ZED
	m_pZED = NULL;
#endif
	m_pMavlink = NULL;
	m_pSF40 = NULL;
	m_initLL.init();
	m_LL.init();
	m_UTM.init();
	m_mavDSfreq = 30;
	m_tStarted = 0;
	m_tNow = 0;
	m_apmMode = 0;

	m_vT.init();
	m_vRot.init();
}

_GPS::~_GPS()
{
}

bool _GPS::init(void* pKiss)
{
	IF_F(!_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("mavDSfreq", &m_mavDSfreq));

	Kiss* pI = pK->o("initLL");
	IF_T(pI->empty());
	F_INFO(pI->v("lat", &m_initLL.m_lat));
	F_INFO(pI->v("lng", &m_initLL.m_lng));
	F_INFO(pI->v("hdg", &m_initLL.m_hdg));

	m_tStarted = getTimeUsec();

	reset();
	return true;
}

bool _GPS::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName;

	iName = "";
	F_INFO(pK->v("_Lightware_SF40", &iName));
	m_pSF40 = (_Lightware_SF40*) (pK->root()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_Mavlink", &iName));
	m_pMavlink= (_Mavlink*) (pK->root()->getChildInstByName(&iName));

#ifdef USE_ZED
	iName = "";
	F_INFO(pK->v("_ZED", &iName));
	m_pZED = (_ZED*) (pK->root()->getChildInstByName(&iName));
#endif

	return true;
}

void _GPS::reset(void)
{
	setLL(&m_initLL);
	m_initUTM = *getUTM();
}

bool _GPS::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _GPS::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		m_tNow = getTimeUsec();
		detect();

		this->autoFPSto();
	}
}

void _GPS::detect(void)
{
	if(m_pMavlink)
	{
		getMavGPS();

		//reset init pos in mode change
		uint32_t apmMode = m_pMavlink->m_msg.heartbeat.custom_mode;
		if(apmMode == 0)apmMode = m_apmMode;
		if(apmMode != m_apmMode)
		{
			m_apmMode = apmMode;
			setLL(&m_initLL);

			if(m_pSF40)
				m_pSF40->reset();

			LOG_I("ZED TRACKING START: APM mode: " + i2str(m_apmMode));
		}
	}

	UTM_POS utm = *getUTM();
	vDouble3 dT = m_vT * (double)this->m_dTime;
	vDouble3 dRot = m_vRot * (double)this->m_dTime;
	double hdgRad = utm.m_hdg * DEG_RAD;
	double sinH = sin(hdgRad);
	double cosH = cos(hdgRad);
	vDouble3 dPos;

	if(m_pSF40)
	{
		//estimate position
//		m_pSF40->setHeading(m_LL.m_hdg);
//		vDouble2 dPos = m_pSF40->getPosDiff();
	}
#ifdef USE_ZED
	else if(m_pZED)
	{
		vDouble3 dM;
		vDouble3 dR;
		uint64_t dTime;
		m_pZED->getMotionDelta(&dM,&dR,&dTime);
		dM.x = constrain(dM.x, -dT.x, dT.x);	//Siding
		dM.y = constrain(dM.y, -dT.y, dT.y);	//Alt
		dM.z = constrain(dM.z, -dT.z, dT.z);	//Heading

		dPos.x = dM.x * cosH + dM.z * sinH;	//Easting
		dPos.y = dM.y;							//Alt
		dPos.z = dM.z * cosH - dM.x * sinH;	//Northing
	}
#endif
	else
	{
		//purely using rpm to identify position translation
		dPos = dT;
	}

	utm.m_easting += dPos.x;
	utm.m_northing += dPos.z;
	utm.m_alt += dPos.y;
	if(!m_pMavlink)
		utm.m_hdg += dRot.x;

	setUTM(&utm);

	if(m_pMavlink)
	{
		setMavGPS();
	}
}

void _GPS::setSpeed(vDouble3* pDT, vDouble3* pDRot)
{
	//dT: m/s in xyz
	//dRot: rad/s in ypr

	if(pDT)m_vT = *pDT;
	if(pDRot)m_vRot = *pDRot;
}

void _GPS::setMavGPS(void)
{
	/*
	time_week
	time_week_ms
	lat
	lon
	alt (optional)
	hdop (optinal)
	vdop (optinal)
	vn, ve, vd (optional)
	speed_accuracy (optional)
	horizontal_accuracy (optional)
	satellites_visible <-- required
	fix_type <-- required
	 */

	NULL_(m_pMavlink);

	mavlink_gps_input_t mavGPS;
	mavGPS.lat = m_LL.m_lat * 1e7;
	mavGPS.lon = m_LL.m_lng * 1e7;
	mavGPS.alt = 0.0;
	mavGPS.gps_id = 0;
	mavGPS.fix_type = 3;
	mavGPS.satellites_visible = 10;
	mavGPS.time_week = 1;
	mavGPS.time_week_ms = (m_tNow-m_tStarted) / 1000;
	mavGPS.ignore_flags = 0b11111111;

	m_pMavlink->gpsInput(&mavGPS);
}

void _GPS::getMavGPS(void)
{
	NULL_(m_pMavlink);

	if(m_tNow - m_pMavlink->m_msg.time_stamps.global_position_int > USEC_1SEC)
	{
		m_pMavlink->requestDataStream(MAV_DATA_STREAM_POSITION, m_mavDSfreq);
		m_pMavlink->requestDataStream(MAV_DATA_STREAM_EXTRA1, m_mavDSfreq);
		return;
	}

	m_LL.m_hdg = ((double)m_pMavlink->m_msg.global_position_int.hdg) * 0.01;
	setLL(&m_LL);

	LOG_I("hdg:"<<m_LL.m_hdg);
}

void _GPS::setLL(LL_POS* pLL)
{
	NULL_(pLL);
	m_LL = *pLL;

	char pUTMzone[8];
	UTM::LLtoUTM(m_LL.m_lat, m_LL.m_lng, m_UTM.m_northing, m_UTM.m_easting, pUTMzone);
	m_UTM.m_zone = pUTMzone;
	m_UTM.m_alt = m_LL.m_alt;
	m_UTM.m_hdg = m_LL.m_hdg;
}

void _GPS::setUTM(UTM_POS* pUTM)
{
	NULL_(pUTM);
	m_UTM = *pUTM;

	UTM::UTMtoLL(m_UTM.m_northing, m_UTM.m_easting, m_UTM.m_zone.c_str(), m_LL.m_lat, m_LL.m_lng);
	m_LL.m_alt = m_UTM.m_alt;
	m_LL.m_hdg = m_UTM.m_hdg;
}

LL_POS* _GPS::getLL(void)
{
	return &m_LL;
}

UTM_POS* _GPS::getUTM(void)
{
	return &m_UTM;
}

LL_POS* _GPS::getInitLL(void)
{
	return &m_initLL;
}

UTM_POS* _GPS::getInitUTM(void)
{
	return &m_initUTM;
}

bool _GPS::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	string msg;

	double dX = m_UTM.m_easting - m_initUTM.m_easting;
	double dY = m_UTM.m_northing - m_initUTM.m_northing;
	double dZ = m_UTM.m_alt - m_initUTM.m_alt;

	pWin->tabNext();
	msg = "Pos: lat=" + f2str(m_LL.m_lat) + ", lng=" + f2str(m_LL.m_lng) + ", alt=" + f2str(m_LL.m_alt) + ", hdg=" + f2str(m_LL.m_hdg);
	pWin->addMsg(&msg);
	msg = "Dist: X=" + f2str(dX) + ", Y=" + f2str(dY) + ", Z=" + f2str(dZ);
	pWin->addMsg(&msg);
	pWin->tabPrev();

	return true;
}

}

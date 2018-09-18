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
	m_pMavlink = NULL;
	m_initLL.init();
	m_LL.init();
	m_initUTM.init();
	m_UTM.init();
	m_vDpos.init();
	m_mavDSfreq = 30;
}

_GPS::~_GPS()
{
}

bool _GPS::init(void* pKiss)
{
	IF_F(!_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("mavDSfreq", &m_mavDSfreq);

	Kiss* pI = pK->o("initLL");
	IF_T(pI->empty());
	pI->v("lat", &m_initLL.m_lat);
	pI->v("lng", &m_initLL.m_lng);
	pI->v("hdg", &m_initLL.m_hdg);

	setLL(&m_initLL);
	m_initUTM = *getUTM();

	//link
	string iName;
	iName = "";
	F_ERROR_F(pK->v("_Mavlink", &iName));
	m_pMavlink = (_Mavlink*) (pK->root()->getChildInst(iName));

	return true;
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

		setMavGPS();

		this->autoFPSto();
	}
}

void _GPS::setMavGPS(void)
{
	NULL_(m_pMavlink);
	IF_(!getMavHdg());

	double hdgRad = m_LL.m_hdg * DEG_RAD;
	double sinH = sin(hdgRad);
	double cosH = cos(hdgRad);

//	dPos.x = dM.x * cosH + dM.z * sinH;	//Easting dM.x=Right,dM.z=Forward
//	dPos.y = dM.y;						//Alt
//	dPos.z = dM.z * cosH - dM.x * sinH;	//Northing

	UTM_POS pUTM = m_initUTM;
	pUTM.m_easting += m_vDpos.y * cosH + m_vDpos.x * sinH;
	pUTM.m_northing += m_vDpos.x * cosH - m_vDpos.y * sinH;
	pUTM.m_alt += m_vDpos.z;
	pUTM.m_hdg = m_UTM.m_hdg;
	setUTM(&pUTM);

	mavlink_gps_input_t D;
	D.lat = m_LL.m_lat * 1e7;
	D.lon = m_LL.m_lng * 1e7;
	D.alt = m_LL.m_alt;
	D.gps_id = 0;
	D.fix_type = 3;
	D.satellites_visible = 10;
	D.ignore_flags = 0b11111111;
	m_pMavlink->gpsInput(D);
}

bool _GPS::getMavHdg(void)
{
	NULL_F(m_pMavlink);

	if(getTimeUsec() - m_pMavlink->m_msg.time_stamps.global_position_int > USEC_1SEC)
	{
		m_pMavlink->requestDataStream(MAV_DATA_STREAM_POSITION, m_mavDSfreq);
		m_pMavlink->requestDataStream(MAV_DATA_STREAM_EXTRA1, m_mavDSfreq);
		return true;
	}

	m_LL.m_hdg = ((double)m_pMavlink->m_msg.global_position_int.hdg) * 0.01;
	setLL(&m_LL);

	LOG_I("hdg: " + i2str(m_LL.m_hdg));

	return true;
}

void _GPS::setRelPos(vDouble3& dPos)
{
	m_vDpos = dPos;
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
	Mat* pMat = pWin->getFrame()->m();
	string msg;

	double dE = m_UTM.m_easting - m_initUTM.m_easting;
	double dN = m_UTM.m_northing - m_initUTM.m_northing;
	double dA = m_UTM.m_alt - m_initUTM.m_alt;

	pWin->tabNext();

	msg = "Pos: lat=" + f2str(m_LL.m_lat*10e7) + ", lng=" + f2str(m_LL.m_lng*10e7) + ", alt=" + f2str(m_LL.m_alt) + ", hdg=" + f2str(m_LL.m_hdg);
	pWin->addMsg(&msg);
	msg = "Dist: E=" + f2str(dE) + ", N=" + f2str(dN) + ", A=" + f2str(dA);
	pWin->addMsg(&msg);

	pWin->tabPrev();

	return true;
}

bool _GPS::cli(int& iY)
{
	IF_F(!this->_ThreadBase::cli(iY));

	double dE = m_UTM.m_easting - m_initUTM.m_easting;
	double dN = m_UTM.m_northing - m_initUTM.m_northing;
	double dA = m_UTM.m_alt - m_initUTM.m_alt;

	string msg;
	msg = "Pos: lat=" + f2str(m_LL.m_lat*10e7) + ", lng=" + f2str(m_LL.m_lng*10e7) + ", alt=" + f2str(m_LL.m_alt) + ", hdg=" + f2str(m_LL.m_hdg);
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	msg = "Dist: E=" + f2str(dE) + ", N=" + f2str(dN) + ", A=" + f2str(dA);
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}

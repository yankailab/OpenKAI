#include "../ArduPilot/_AP_GPS.h"

#ifdef USE_REALSENSE

namespace kai
{

_AP_GPS::_AP_GPS()
{
	m_pAP = NULL;
	m_pRS = NULL;

	m_yaw = 0.0;
	m_dYaw = 0.0;
	m_utmPos.init();

	m_bUseApOrigin = false;
	m_llOrigin.init();
}

_AP_GPS::~_AP_GPS()
{
}

bool _AP_GPS::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("lat", &m_llOrigin.m_lat);
	pK->v("lng", &m_llOrigin.m_lng);
	m_utmOrigin = m_GPS.LL2UTM(m_llOrigin);

	pK->v("gpsID",&m_D.gps_id);
	pK->v("iFixType",&m_D.fix_type);
	pK->v("nSat",&m_D.satellites_visible);
	pK->v("hdop",&m_D.hdop);
	pK->v("vdop",&m_D.vdop);

	string iName;

	iName = "";
	F_ERROR_F(pK->v("_AP_base", &iName));
	m_pAP = (_AP_base*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	F_ERROR_F(pK->v("_RStracking", &iName));
	m_pRS = (_RStracking*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pRS, iName + ": not found");

	return true;
}

bool _AP_GPS::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< "Return code: "<< retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _AP_GPS::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMav,-1);
	NULL__(m_pRS,-1);

	return 0;
}

void _AP_GPS::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();
		this->_AutopilotBase::update();

		updateGPS();

		this->autoFPSto();
	}
}

void _AP_GPS::updateGPS(void)
{
	IF_(check()<0);

	if(m_bUseApOrigin)
	{
		vDouble3 vHome = m_pAP->getHomePos();
		IF_(vHome.x < 0.0);

		m_llOrigin.m_lat = vHome.x;
		m_llOrigin.m_lng = vHome.y;
		m_utmOrigin = m_GPS.LL2UTM(m_llOrigin);
	}

//	m_dYaw = m_pSB->q();
	m_utmOrigin.m_hdg = m_yaw + m_dYaw;
	m_utmOrigin.m_altRel = ((double)m_pAP->m_pMav->m_globalPositionINT.m_msg.relative_alt) * 0.01;

	vFloat3 vRSt = m_pRS->t();
	vFloat3 vT;
	vT.x = vRSt.x;
	vT.y = vRSt.y;
	vT.z = vRSt.z;
	UTM_POS pUTM = m_GPS.offset(m_utmOrigin, vT);
	m_llPos = m_GPS.UTM2LL(pUTM);

	double tBase = (double)USEC_1SEC/(double)m_dTime;
	vFloat3 vV;
	vV.x = (pUTM.m_northing - m_utmPos.m_northing)*tBase;
	vV.y = (pUTM.m_easting - m_utmPos.m_easting)*tBase;
	vV.z = (pUTM.m_altRel - m_utmPos.m_altRel)*tBase;
	m_utmPos = pUTM;

	m_D.lat = m_llPos.m_lat * 1e7;
	m_D.lon = m_llPos.m_lng * 1e7;
	m_D.alt = (float)m_llPos.m_altRel;
	m_D.vn = (float)vV.x;
	m_D.ve = (float)vV.y;
	m_D.vd = (float)vV.z;
	m_D.ignore_flags = 0b11110111;
	m_pAP->m_pMav->gpsInput(m_D);

}

void _AP_GPS::reset(void)
{
	IF_(check()<0);

	m_pRS->reset();
	m_yaw = ((double)m_pAP->m_pMav->m_globalPositionINT.m_msg.hdg) * 0.01;
	m_dYaw = 0.0;
}

void _AP_GPS::draw(void)
{
	this->_AutopilotBase::draw();

	addMsg("lat=" + lf2str(m_llPos.m_lat,7) + ", lon=" + lf2str(m_llPos.m_lng,7));
	addMsg("yaw=" + f2str(m_yaw) + ", dYaw = " + f2str(m_dYaw));

}

}

#endif


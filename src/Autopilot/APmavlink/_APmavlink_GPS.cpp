#include "_APmavlink_GPS.h"

namespace kai
{

	_APmavlink_GPS::_APmavlink_GPS()
	{
		m_pAP = nullptr;
		m_pSB = nullptr;

		m_yaw = 0.0;
		m_bYaw = false;
		m_iRelayLED = 0;

		m_vAxisIdx.set(0, 1, 2);
		m_vAxisK.set(1, 1, 1);
		m_utmPos.init();

		m_bUseApOrigin = false;
		m_llOrigin.init();

		m_D.gps_id = 0;
		m_D.fix_type = 3;
		m_D.satellites_visible = 10;
		m_D.hdop = 0;
		m_D.vdop = 0;
		m_D.ignore_flags = 0b11110101;
	}

	_APmavlink_GPS::~_APmavlink_GPS()
	{
	}

	int _APmavlink_GPS::init(void *pKiss)
	{
		CHECK(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;


		pK->v("yaw", &m_yaw);
		pK->v("bYaw", &m_bYaw);
		pK->v("iRelayLED", &m_iRelayLED);

		pK->v("vAxisIdx", &m_vAxisIdx);
		pK->v("vAxisK", &m_vAxisK);
		pK->v("lat", &m_llOrigin.m_lat);
		pK->v("lng", &m_llOrigin.m_lng);
		m_utmOrigin = m_GPS.LL2UTM(m_llOrigin);

		pK->v("gpsID", &m_D.gps_id);
		pK->v("iFixType", &m_D.fix_type);
		pK->v("nSat", &m_D.satellites_visible);
		pK->v("hdop", &m_D.hdop);
		pK->v("vdop", &m_D.vdop);
		pK->v("fIgnore", &m_D.ignore_flags);

		string n;

		n = "";
		pK->v("_APmavlink_base", &n);
		m_pAP = (_APmavlink_base *)(pK->findModule(n));
		NULL__(m_pAP, OK_ERR_NOT_FOUND);

		n = "";
		pK->v("_NavBase", &n);
		m_pSB = (_NavBase *)(pK->findModule(n));
		NULL_(m_pSB, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	int _APmavlink_GPS::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _APmavlink_GPS::check(void)
	{
		NULL__(m_pAP, OK_ERR_NULLPTR);
		NULL__(m_pAP->getMavlink(), OK_ERR_NULLPTR);
		NULL__(m_pSB, OK_ERR_NULLPTR);

		return this->_ModuleBase::check();
	}

	void _APmavlink_GPS::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();
			this->_ModuleBase::update();

			updateGPS();

		}
	}

	void _APmavlink_GPS::updateGPS(void)
	{
		IF_(check() != OK_OK);

		//	m_scApArm.update(m_pAP->bApArmed());
		//	if(m_scApArm.bActive(true))
		//		m_bYaw = false;

		if (!m_bYaw)
		{
			m_bYaw = reset();
			return;
		}

		m_pAP->getMavlink()->clDoSetRelay(m_iRelayLED, true);

		if (m_bUseApOrigin)
		{
			vDouble3 vHome = m_pAP->getHomePos();
			IF_(vHome.x < 0.0);

			m_llOrigin.m_lat = vHome.x;
			m_llOrigin.m_lng = vHome.y;
			m_utmOrigin = m_GPS.LL2UTM(m_llOrigin);
		}

		m_utmOrigin.m_hdg = m_yaw;
		m_utmOrigin.m_altRel = ((double)m_pAP->getMavlink()->m_globalPositionINT.m_msg.relative_alt) * 0.01;

		vFloat3 vRSt = m_pSB->t();
		vFloat3 vT;
		vT.x = *vRSt.v(m_vAxisIdx.x) * m_vAxisK.x;
		vT.y = *vRSt.v(m_vAxisIdx.y) * m_vAxisK.y;
		vT.z = *vRSt.v(m_vAxisIdx.z) * m_vAxisK.z;
		UTM_POS pUTM = m_GPS.offset(m_utmOrigin, vT);
		m_llPos = m_GPS.UTM2LL(pUTM);

		double tBase = (double)SEC_2_USEC / (double)m_pT->getDt();
		vFloat3 vV;
		vV.x = (pUTM.m_northing - m_utmPos.m_northing) * tBase;
		vV.y = (pUTM.m_easting - m_utmPos.m_easting) * tBase;
		vV.z = (pUTM.m_altRel - m_utmPos.m_altRel) * tBase;
		m_utmPos = pUTM;

		m_D.lat = m_llPos.m_lat * 1e7;
		m_D.lon = m_llPos.m_lng * 1e7;
		m_D.alt = (float)m_llPos.m_altRel;
		m_D.vn = vV.x;
		m_D.ve = vV.y;
		m_D.vd = vV.z;
		m_pAP->getMavlink()->gpsInput(m_D);
	}

	bool _APmavlink_GPS::reset(void)
	{
		IF_F(check() != OK_OK);

		uint16_t hdg = m_pAP->getMavlink()->m_globalPositionINT.m_msg.hdg;
		IF_F(hdg == UINT16_MAX);

		m_yaw = ((double)hdg) * 0.01;
		//	m_pSB->reset();

		return true;
	}

	void _APmavlink_GPS::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		((_Console *)pConsole)->addMsg("lat=" + lf2str(m_llPos.m_lat, 7) + ", lon=" + lf2str(m_llPos.m_lng, 7) + ", yaw=" + f2str(m_yaw));
	}

}

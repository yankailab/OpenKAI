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

	bool _APmavlink_GPS::init(const json &j)
	{
		IF_F(!this->_ModuleBase::init(j));

		jVar(j, "yaw", m_yaw);
		jVar(j, "bYaw", m_bYaw);
		jVar(j, "iRelayLED", m_iRelayLED);

		jVec<int>(j, "vAxisIdx", m_vAxisIdx);
		jVec<float>(j, "vAxisK", m_vAxisK);
		jVar(j, "lat", m_llOrigin.m_lat);
		jVar(j, "lng", m_llOrigin.m_lng);
		m_utmOrigin = m_GPS.LL2UTM(m_llOrigin);

		jVar(j, "gpsID", m_D.gps_id);
		jVar(j, "iFixType", m_D.fix_type);
		jVar(j, "nSat", m_D.satellites_visible);
		jVar(j, "hdop", m_D.hdop);
		jVar(j, "vdop", m_D.vdop);
		jVar(j, "fIgnore", m_D.ignore_flags);

		return true;
	}

	bool _APmavlink_GPS::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		string n;

		n = "";
		jVar(j, "_APmavlink_base", n);
		m_pAP = (_APmavlink_base *)(pM->findModule(n));
		NULL_F(m_pAP);

		n = "";
		jVar(j, "_NavBase", n);
		m_pSB = (_NavBase *)(pM->findModule(n));
		NULL_(m_pSB);

		return true;
	}

	bool _APmavlink_GPS::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _APmavlink_GPS::check(void)
	{
		NULL_F(m_pAP);
		IF_F(!m_pAP->getMavlink());
		NULL_F(m_pSB);

		return this->_ModuleBase::check();
	}

	void _APmavlink_GPS::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateGPS();
		}
	}

	void _APmavlink_GPS::updateGPS(void)
	{
		IF_(!check());

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
		IF_F(!check());

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

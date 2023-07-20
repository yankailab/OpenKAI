#include "../ArduPilot/_AP_move.h"

namespace kai
{

	_AP_move::_AP_move()
	{
		m_pAP = NULL;

		m_vPvar.clear();
		m_vPsp.clear();
		// m_vPsp.x = 0.5;
		// m_vPsp.y = 0.5;
		m_vPspGlobal.clear();
		m_vPspGlobal.z = 10.0;

		m_pPitch = NULL;
		m_pRoll = NULL;
		m_pAlt = NULL;
		m_pYaw = NULL;

		m_vKpidIn.set(0.0, 1.0);
		m_vKpidOut.set(0.0, 1.0);
		m_vKpid.set(1.0);

		m_sptLocal.vx = 0.0;
		m_sptLocal.vy = 0.0;
		m_sptLocal.vz = 0.0;
		m_sptLocal.x = 0.0;
		m_sptLocal.y = 0.0;
		m_sptLocal.z = 0.0;
		m_sptLocal.yaw = 0.0;
		m_sptLocal.yaw_rate = 0.0;

		m_sptGlobal.vx = 0.0;
		m_sptGlobal.vy = 0.0;
		m_sptGlobal.vz = 0.0;
		m_sptGlobal.lat_int = 0.0;
		m_sptGlobal.lon_int = 0.0;
		m_sptGlobal.alt = 0.0;
		m_sptGlobal.yaw = 0.0;
		m_sptGlobal.yaw_rate = 90.0 * DEG_2_RAD;
	}

	_AP_move::~_AP_move()
	{
	}

	bool _AP_move::init(void *pKiss)
	{
		IF_F(!this->_StateBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vPsp", &m_vPsp);
		pK->v("vPspGlobal", &m_vPspGlobal);
		pK->v("vKpidIn", &m_vKpidIn);
		pK->v("vKpidOut", &m_vKpidOut);

		return true;
	}

	bool _AP_move::link(void)
	{
		IF_F(!this->_StateBase::link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		n = "";
		pK->v("PIDpitch", &n);
		m_pPitch = (PID *)(pK->getInst(n));

		n = "";
		pK->v("PIDroll", &n);
		m_pRoll = (PID *)(pK->getInst(n));

		n = "";
		pK->v("PIDalt", &n);
		m_pAlt = (PID *)(pK->getInst(n));

		n = "";
		pK->v("PIDyaw", &n);
		m_pYaw = (PID *)(pK->getInst(n));

		n = "";
		pK->v("_AP_base", &n);
		m_pAP = (_AP_base *)(pK->getInst(n));
		IF_Fl(!m_pAP, n + ": not found");

		return true;
	}

	bool _AP_move::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _AP_move::check(void)
	{
		NULL__(m_pAP, -1);
		NULL__(m_pAP->m_pMav, -1);

		return this->_StateBase::check();
	}

	void _AP_move::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();
			this->_StateBase::update();

			setPosLocal();

			m_pT->autoFPSto();
		}
	}

	void _AP_move::setPosLocal(uint8_t frame, bool bP, bool bV, bool bA, bool bYaw, bool bYawRate)
	{
		IF_(check() < 0);

		float dTs = m_pT->getDt() * USEC_2_SEC;
		float p = 0, r = 0, a = 0, y = 0;

		if (m_pPitch)
			p = m_pRoll->update(m_vPvar.x, m_vPsp.x, dTs);

		if (m_pRoll)
			r = m_pPitch->update(m_vPvar.y, m_vPsp.y, dTs);

		if (m_pAlt)
			a = m_pAlt->update(m_vPvar.z, m_vPsp.z, dTs);
		else
			a = m_vPvar.z;

		if (m_pYaw)
			y = m_pYaw->update(dHdg<float>(m_vPsp.w, m_vPvar.w), 0.0, dTs);

		p *= m_vKpid.x;
		r *= m_vKpid.y;
		a *= m_vKpid.z;
		y *= m_vKpid.w;

		m_sptLocal.coordinate_frame = frame;
		m_sptLocal.vx = p; // forward
		m_sptLocal.vy = r; // right
		m_sptLocal.vz = a; // down
		m_sptLocal.yaw = 0.0;
		m_sptLocal.yaw_rate = y * DEG_2_RAD;

		uint16_t tm = 0;
		tm = (!bP) ? (tm | (0b111)) : tm;
		tm = (!bV) ? (tm | (0b111 << 3)) : tm;
		tm = (!bA) ? (tm | (0b111 << 6)) : tm;
		tm = (!bYaw) ? (tm | (1 << 10)) : tm;
		tm = (!bYawRate) ? (tm | (1 << 11)) : tm;
		m_sptLocal.type_mask = tm;

		m_pAP->m_pMav->setPositionTargetLocalNED(m_sptLocal);
	}

	void _AP_move::setPosGlobal(uint8_t frame, bool bP, bool bV, bool bA, bool bYaw, bool bYawRate)
	{
		IF_(check() < 0);

		m_sptGlobal.coordinate_frame = frame;
		m_sptGlobal.lat_int = m_vPspGlobal.x * 1e7;
		m_sptGlobal.lon_int = m_vPspGlobal.y * 1e7;
		m_sptGlobal.alt = m_vPspGlobal.z;
		m_sptGlobal.vx = 0.0;
		m_sptGlobal.vy = 0.0;
		m_sptGlobal.vz = 0.0;
		m_sptGlobal.yaw = (float)m_vPspGlobal.w * DEG_2_RAD;

		uint16_t tm = 0;
		tm = (!bP) ? (tm | (0b111)) : tm;
		tm = (!bV) ? (tm | (0b111 << 3)) : tm;
		tm = (!bA) ? (tm | (0b111 << 6)) : tm;
		tm = (!bYaw) ? (tm | (1 << 10)) : tm;
		tm = (!bYawRate) ? (tm | (1 << 11)) : tm;
		m_sptGlobal.type_mask = tm;
//		m_sptGlobal.type_mask = 0b0000000111111000;

		m_pAP->m_pMav->setPositionTargetGlobalINT(m_sptGlobal);
	}

	void _AP_move::clearPID(void)
	{
		if (m_pPitch)
			m_pPitch->reset();
		if (m_pRoll)
			m_pRoll->reset();
		if (m_pAlt)
			m_pAlt->reset();
		if (m_pYaw)
			m_pYaw->reset();
	}

	void _AP_move::stop(void)
	{
		IF_(check() < 0);

		m_sptLocal.coordinate_frame = MAV_FRAME_BODY_OFFSET_NED;
		m_sptLocal.x = 0.0;
		m_sptLocal.y = 0.0;
		m_sptLocal.z = 0.0;
		m_sptLocal.vx = 0;
		m_sptLocal.vy = 0;
		m_sptLocal.vz = 0;
		m_sptLocal.yaw_rate = 0.0;
		m_sptLocal.type_mask = 0b0000010111000111;
		m_pAP->m_pMav->setPositionTargetLocalNED(m_sptLocal);
	}

	void _AP_move::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_StateBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("Local NED:");
		pC->addMsg("vPsp = (" + f2str(m_vPsp.x) + ", " + f2str(m_vPsp.y) + ", " + f2str(m_vPsp.z) + ", " + f2str(m_vPsp.w) + ")", 1);
		pC->addMsg("vPvar = (" + f2str(m_vPvar.x) + ", " + f2str(m_vPvar.y) + ", " + f2str(m_vPvar.z) + ", " + f2str(m_vPvar.w) + ")", 1);
		pC->addMsg("setV = (" + f2str(m_sptLocal.vx, 7) + ", " + f2str(m_sptLocal.vy, 7) + ", " + f2str(m_sptLocal.vz, 7) + ")", 1);
		pC->addMsg("setP = (" + f2str(m_sptLocal.x, 7) + ", " + f2str(m_sptLocal.y, 7) + ", " + f2str(m_sptLocal.z, 7) + ")", 1);
		pC->addMsg("yawRate=" + f2str(m_sptLocal.yaw_rate) + ", yaw=" + f2str(m_sptLocal.yaw), 1);

		pC->addMsg("Global INT:");
		pC->addMsg("vTargetGlobal = (" + lf2str(m_vPspGlobal.x, 7) + ", " + lf2str(m_vPspGlobal.y, 7) + ", " + lf2str(m_vPspGlobal.z, 2) + ", " + lf2str(m_vPspGlobal.w, 2) + ")", 1);
	}

}

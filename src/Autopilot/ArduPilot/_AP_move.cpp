#include "../ArduPilot/_AP_move.h"

namespace kai
{

	_AP_move::_AP_move()
	{
		m_pAP = NULL;
	}

	_AP_move::~_AP_move()
	{
	}

	bool _AP_move::init(void *pKiss)
	{
		IF_F(!this->_StateBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		return true;
	}

	bool _AP_move::link(void)
	{
		IF_F(!this->_StateBase::link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;

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

			m_pT->autoFPSto();
		}
	}

	void _AP_move::stop(void)
	{
		IF_(check() < 0);

		mavlink_set_position_target_local_ned_t spt;
		spt.coordinate_frame = MAV_FRAME_BODY_OFFSET_NED;
		spt.x = 0.0;
		spt.y = 0.0;
		spt.z = 0.0;
		spt.vx = 0;
		spt.vy = 0;
		spt.vz = 0;
		spt.yaw_rate = 0.0;
		spt.type_mask = 0b0000010111000111;
		m_pAP->m_pMav->setPositionTargetLocalNED(spt);
	}

	void _AP_move::setHdg(float y, float r, bool bYaw, bool bYawRate, uint8_t frame)
	{
		IF_(check() < 0);

		mavlink_set_position_target_local_ned_t spt;
		spt.coordinate_frame = frame;
		spt.yaw = y;
		spt.yaw_rate = r;
		spt.type_mask = 0b0000000111111111; // AVP
		if (!bYaw)
			spt.type_mask |= (1 << 10);
		if (!bYawRate)
			spt.type_mask |= (1 << 11);

		m_pAP->m_pMav->setPositionTargetLocalNED(spt);
	}

	void _AP_move::setVlocal(const vFloat4 &vSpd, bool bYaw, bool bYawRate, uint8_t frame)
	{
		IF_(check() < 0);

		mavlink_set_position_target_local_ned_t spt;
		spt.coordinate_frame = frame;
		spt.vx = vSpd.x; // forward
		spt.vy = vSpd.y; // right
		spt.vz = vSpd.z; // down
		spt.yaw = 0.0;
		spt.yaw_rate = vSpd.w;
		spt.type_mask = 0b0000000111000111; // AVP
		if (!bYaw)
			spt.type_mask |= (1 << 10);
		if (!bYawRate)
			spt.type_mask |= (1 << 11);

		m_pAP->m_pMav->setPositionTargetLocalNED(spt);
	}

	void _AP_move::setPglobal(const vDouble4 &vP, bool bYaw, bool bYawRate, uint8_t frame)
	{
		IF_(check() < 0);

		mavlink_set_position_target_global_int_t spt;
		spt.coordinate_frame = frame;
		spt.lat_int = vP.x * 1e7;
		spt.lon_int = vP.y * 1e7;
		spt.alt = vP.z;
		spt.vx = 0.0;
		spt.vy = 0.0;
		spt.vz = 0.0;
		spt.yaw = vP.w;
		spt.type_mask = 0b0000000111111000; // AVP
		if (!bYaw)
			spt.type_mask |= (1 << 10);
		if (!bYawRate)
			spt.type_mask |= (1 << 11);

		m_pAP->m_pMav->setPositionTargetGlobalINT(spt);
	}

	void _AP_move::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_StateBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		//		pC->addMsg("Local NED:");
	}

}

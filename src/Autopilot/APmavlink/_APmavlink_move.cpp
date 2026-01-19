#include "_APmavlink_move.h"

namespace kai
{

	_APmavlink_move::_APmavlink_move()
	{
		m_pAP = nullptr;
	}

	_APmavlink_move::~_APmavlink_move()
	{
	}

	bool _APmavlink_move::init(const json &j)
	{
		IF_F(!this->_ModuleBase::init(j));

		return true;
	}

	bool _APmavlink_move::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		string n = "";
		jVar(j, "_APmavlink_base", n);
		m_pAP = (_APmavlink_base *)(pM->findModule(n));
		NULL_F(m_pAP);

		return true;
	}

	bool _APmavlink_move::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _APmavlink_move::check(void)
	{
		NULL_F(m_pAP);
		IF_F(!m_pAP->getMavlink());

		return this->_ModuleBase::check();
	}

	void _APmavlink_move::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();
		}
	}

	void _APmavlink_move::setHold(void)
	{
		IF_(!check());

		mavlink_set_position_target_local_ned_t spt;
		spt.coordinate_frame = MAV_FRAME_BODY_OFFSET_NED;
		spt.x = 0.0;
		spt.y = 0.0;
		spt.z = 0.0;
		spt.vx = 0;
		spt.vy = 0;
		spt.vz = 0;
		spt.yaw = 0.0;
		spt.yaw_rate = 0.0;
		spt.type_mask = 0b0000010111000111;
		m_pAP->getMavlink()->setPositionTargetLocalNED(spt);
	}

	void _APmavlink_move::setHdg(float y, float r, bool bYaw, bool bYawRate, uint8_t frame)
	{
		IF_(!check());

		mavlink_set_position_target_local_ned_t spt;
		spt.coordinate_frame = frame;
		spt.x = 0.0;
		spt.y = 0.0;
		spt.z = 0.0;
		spt.vx = 0;
		spt.vy = 0;
		spt.vz = 0;
		spt.yaw = y;
		spt.yaw_rate = r;
		spt.type_mask = 0b0000000111000111; // AVP
		if (!bYaw)
			spt.type_mask |= IGN_YAW;
		if (!bYawRate)
			spt.type_mask |= IGN_YAW_RATE;

		m_pAP->getMavlink()->setPositionTargetLocalNED(spt);
	}

	void _APmavlink_move::setVlocal(const vFloat4 &vSpd, bool bYaw, bool bYawRate, uint8_t frame)
	{
		IF_(!check());

		mavlink_set_position_target_local_ned_t spt;
		spt.coordinate_frame = frame;
		spt.vx = vSpd.x; // forward
		spt.vy = vSpd.y; // right
		spt.vz = vSpd.z; // down
		spt.yaw = 0.0;
		spt.yaw_rate = vSpd.w;
		spt.type_mask = 0b0000000111000111; // AVP
		if (!bYaw)
			spt.type_mask |= IGN_YAW;
		if (!bYawRate)
			spt.type_mask |= IGN_YAW_RATE;

		m_pAP->getMavlink()->setPositionTargetLocalNED(spt);
	}

	void _APmavlink_move::setPlocal(const vFloat4 &vP, bool bYaw, bool bYawRate, uint8_t frame)
	{
		IF_(!check());

		mavlink_set_position_target_local_ned_t spt;
		spt.coordinate_frame = frame;
		spt.x = vP.x; // forward
		spt.y = vP.y; // right
		spt.z = vP.z; // down
		spt.yaw = 0.0;
		spt.yaw_rate = vP.w;
		spt.type_mask = 0b0000000111111000; // AVP
		if (!bYaw)
			spt.type_mask |= IGN_YAW;
		if (!bYawRate)
			spt.type_mask |= IGN_YAW_RATE;

		m_pAP->getMavlink()->setPositionTargetLocalNED(spt);
	}

	void _APmavlink_move::setPglobal(const vDouble4 &vP, bool bYaw, bool bYawRate, uint8_t frame)
	{
		IF_(!check());

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
			spt.type_mask |= IGN_YAW;
		if (!bYawRate)
			spt.type_mask |= IGN_YAW_RATE;

		m_pAP->getMavlink()->setPositionTargetGlobalINT(spt);
	}

	void _APmavlink_move::doReposition(const vDouble4 &vP,
									   float speed,
									   float radius,
									   uint8_t frame)
	{
		IF_(!check());

		mavlink_command_int_t D;
		D.frame = frame;
		D.command = MAV_CMD_DO_REPOSITION;
		D.param1 = speed;
		D.param2 = 0;
		D.param3 = radius;
		D.param4 = vP.w;
		D.x = (int32_t)(vP.x * 1e7);
		D.y = (int32_t)(vP.y * 1e7);
		D.z = vP.z;

		m_pAP->getMavlink()->cmdInt(D);
	}

	void _APmavlink_move::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		//		pC->addMsg("Local NED:");
	}

}

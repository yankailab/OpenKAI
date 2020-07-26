#include "_S6H4D.h"

namespace kai
{

_S6H4D::_S6H4D()
{
	m_vNoriginPos.init(0.5);
	m_vNoriginAngle.init(0.5);

	m_pIO = NULL;
	m_state.init();

	m_vXrange.init(-100.0, 100.0);
	m_vYrange.init(-100.0, 100.0);
	m_vZrange.init(-100.0, 100.0);
	m_vA1range.init(0.0, 180.0);
	m_vA2range.init(0.0, 180.0);
	m_vA3range.init(0.0, 180.0);
	m_vSpeedRange.init(0.0, 1e5);
	m_speed = 1.0;

	m_dMove = 128;
	m_bOrder = true;
	m_bAutoGripperHdg = false;
	m_autoGripperHdgSpeed = 0.2;

	m_cState.init();
	m_tState.init();
	m_tState.m_vOrigin.init(344, 1.0, 417);
}

_S6H4D::~_S6H4D()
{
}

bool _S6H4D::init(void *pKiss)
{
	IF_F(!this->_ActuatorBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	pK->v("vXrange", &m_vXrange);
	pK->v("vYrange", &m_vYrange);
	pK->v("vZrange", &m_vZrange);

	pK->v("vA1range", &m_vA1range);
	pK->v("vA2range", &m_vA2range);
	pK->v("vA3range", &m_vA3range);

	pK->v("vSpeedRange", &m_vSpeedRange);
	pK->v("speed", &m_speed);
	pK->v("bAutoGripperHdg", &m_bAutoGripperHdg);
	pK->v("autoGripperHdgSpeed", &m_autoGripperHdgSpeed);

	pK->v("bOrder", &m_bOrder);
	pK->v("mode", &m_tState.m_mode);
	pK->v("vOrigin", &m_tState.m_vOrigin);

	string iName = "";
	F_ERROR_F(pK->v("_IOBase", &iName));
	m_pIO = (_IOBase*) (pK->getInst(iName));
	IF_Fl(!m_pIO, iName + " not found");

	return true;
}

bool _S6H4D::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR) << "Return code: " << retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _S6H4D::check(void)
{
	NULL__(m_pIO, -1);
	IF__(!m_pIO->isOpen(), -1);

	return 0;
}

void _S6H4D::update(void)
{
	while(check() < 0)
		this->sleepTime(USEC_1SEC);

	setMode(m_tState.m_mode);
	if(m_tState.m_vOrigin != m_cState.m_vOrigin)
		setOrigin(m_tState.m_vOrigin);

	//go to the origin
	updatePos();


	while (m_bThreadON)
	{
		this->autoFPSfrom();

		if(m_vNtargetPos.sum() > -4.0)
		{
//			updatePos();
		}

		if(m_vNtargetSpeed.sum() > -4.0)
		{
			updateMove();
		}

		if(m_vNtargetAngle.sum() > -4.0)
		{
//			updateRot();
		}

		readState();

		if(m_bAutoGripperHdg)
			autoGripperHdg();

		this->autoFPSto();
	}
}

void _S6H4D::updatePos(void)
{
	IF_(check() < 0);

	m_tState.m_vP.x = m_vNtargetPos.x * m_vXrange.d() + m_vXrange.x;
	m_tState.m_vP.y = m_vNtargetPos.y * m_vYrange.d() + m_vYrange.x;
	m_tState.m_vP.z = m_vNtargetPos.z * m_vZrange.d() + m_vZrange.x;

	m_tState.m_vA2.x = m_vNtargetAngle.x * m_vA1range.d() + m_vA1range.x;
	m_tState.m_vA2.y = m_vNtargetAngle.y * m_vA2range.d() + m_vA2range.x;
	m_tState.m_vA2.z = m_vNtargetAngle.z * m_vA3range.d() + m_vA3range.x;

	float spd = m_speed * m_vSpeedRange.d() + m_vSpeedRange.x;

	gotoPos(m_tState.m_vP, m_tState.m_vA2, spd);
}

void _S6H4D::updateMove(void)
{
	IF_(check() < 0);

	vFloat3 vM;
	vM.x = m_vNtargetSpeed.x;
	vM.y = m_vNtargetSpeed.y;
	vM.z = m_vNtargetSpeed.z;

	move(vM);
}

void _S6H4D::updateRot(void)
{
	IF_(check() < 0);

	if(m_vNtargetRotSpeed.x >= 0.0)
		rot(3, m_vNtargetRotSpeed.x);

	if(m_vNtargetRotSpeed.y >= 0.0)
		rot(4, m_vNtargetRotSpeed.y);

	if(m_vNtargetRotSpeed.z >= 0.0)
		rot(5, m_vNtargetRotSpeed.z);
}

void _S6H4D::autoGripperHdg(void)
{
	IF_(check() < 0);

//	if(m_cState.m_vA2.z <= m_vRotRangeZ.x || m_cState.m_vA2.z >= m_vRotRangeZ.y)
//	{
//		rot(5, 0.5);
//		return;
//	}

	if(EAQ(m_cState.m_vA1.x, -m_cState.m_vA2.z, 0.1))
		rot(5, 0.5);
	else if(m_cState.m_vA1.x > -m_cState.m_vA2.z)
		rot(5, 0.5 + m_autoGripperHdgSpeed);
	else
		rot(5, 0.5 - m_autoGripperHdgSpeed);
}

void _S6H4D::armReset(void)
{
	IF_(check() < 0);

	S6H4D_CMD_CTRL cmd;
	cmd.init();
	cmd.m_b[1] = 12;
	cmd.m_b[2] = 3;
	m_pIO->write(cmd.m_b, S6H4D_CMD_N);
}

void _S6H4D::setOrigin(vFloat3& vP)
{
	IF_(check() < 0);

	S6H4D_CMD_CTRL cmd;
	cmd.init();
	cmd.m_b[1] = 20;
	cmd.m_b[2] = 1;
	f2b(&cmd.m_b[3], vP.x * 10.0);
	f2b(&cmd.m_b[7], vP.y * 10.0);
	f2b(&cmd.m_b[11], vP.z * 10.0);
	m_pIO->write(cmd.m_b, S6H4D_CMD_N);
}

void _S6H4D::setMode(int mode)
{
	IF_(check() < 0);

	S6H4D_CMD_CTRL cmd;
	cmd.init();
	cmd.m_b[1] = 30;
	cmd.m_b[2] = 9;
	cmd.m_b[3] = (mode == 0)?0:9;
	m_pIO->write(cmd.m_b, S6H4D_CMD_N);
}

void _S6H4D::gotoPos(vFloat3 &vP, vFloat3& vR, float speed)
{
	IF_(check() < 0);

	S6H4D_CMD_MOV cmd;
	cmd.init('1', 0);
	cmd.set(vP, vR, speed);
	m_pIO->write(cmd.m_b, S6H4D_CMD_N);
}

void _S6H4D::move(vFloat3& vM)
{
	IF_(check() < 0);

	S6H4D_CMD_CTRL cmd;
	cmd.init();
	cmd.m_b[1] = 30;
	cmd.m_b[2] = 7;
	cmd.m_b[3] = 100;
	cmd.m_b[4] = (vM.x >= 0.0) ? constrain<float>(128 + (vM.x - 0.5)*2.0*m_dMove, 0, 255) : 128;
	cmd.m_b[5] = (vM.y >= 0.0) ? constrain<float>(128 + (vM.y - 0.5)*2.0*m_dMove, 0, 255) : 128;
	cmd.m_b[6] = (vM.z >= 0.0) ? constrain<float>(128 + (vM.z - 0.5)*2.0*m_dMove, 0, 255) : 128;

	m_pIO->write(cmd.m_b, S6H4D_CMD_N);
}

void _S6H4D::rot(int iAxis, float angle)
{
	IF_(check() < 0);

	S6H4D_CMD_CTRL cmd;
	cmd.init();
	cmd.m_b[1] = 30;
	cmd.m_b[2] = 7;
	cmd.m_b[3] = iAxis;
	cmd.m_b[4] = (angle >= 0.0) ? constrain<float>(128 + (angle - 0.5)*2.0*m_dMove, 0, 255) : 128;

	m_pIO->write(cmd.m_b, S6H4D_CMD_N);
}

void _S6H4D::pause(void)
{
	IF_(check() < 0);

	S6H4D_CMD_CTRL cmd;
	cmd.init();
	cmd.m_b[1] = 30;
	cmd.m_b[2] = 3;
	cmd.m_b[3] = 2;

	m_pIO->write(cmd.m_b, S6H4D_CMD_N);
}

void _S6H4D::readState(void)
{
	IF_(check() < 0);

	uint8_t b;
	int n;
	while ((n = m_pIO->read(&b, 1)) > 0)
	{
		if (m_state.m_iB != 0)
		{
			m_state.m_pB[m_state.m_iB++] = b;

			if(m_state.m_iB == 9)
			{
				decodeState();
				m_state.init();
			}
		}
		else if (b == S6H4D_STAT_BEGIN)
		{
			m_state.m_pB[0] = b;
			m_state.m_iB = 1;
		}
	}
}

void _S6H4D::decodeState(void)
{
	IF_(m_state.m_pB[8] != S6H4D_STAT_END);

	switch (m_state.m_pB[7])
	{
	case 0:
		m_cState.m_vP.x = 0.1 * (float)unpack_int16(&m_state.m_pB[1], m_bOrder) - m_cState.m_vOrigin.x;
		m_cState.m_vP.y = 0.1 * (float)unpack_int16(&m_state.m_pB[3], m_bOrder) - m_cState.m_vOrigin.y;
		m_cState.m_vP.z = 0.1 * (float)unpack_int16(&m_state.m_pB[5], m_bOrder) - m_cState.m_vOrigin.z;

		m_vNpos.x = (float) (m_cState.m_vP.x - m_vXrange.x) / (float) m_vXrange.len();
		m_vNpos.y = (float) (m_cState.m_vP.y - m_vYrange.x) / (float) m_vYrange.len();
		m_vNpos.z = (float) (m_cState.m_vP.z - m_vZrange.x) / (float) m_vZrange.len();
		break;
	case 1:
		m_cState.m_vA1.x = 0.01 * (float)unpack_int16(&m_state.m_pB[1], m_bOrder);
		m_cState.m_vA1.y = 0.01 * (float)unpack_int16(&m_state.m_pB[3], m_bOrder);
		m_cState.m_vA1.z = 0.01 * (float)unpack_int16(&m_state.m_pB[5], m_bOrder);
		break;
	case 2:
		m_cState.m_vA2.x = 0.01 * (float)unpack_int16(&m_state.m_pB[1], m_bOrder);
		m_cState.m_vA2.y = 0.01 * (float)unpack_int16(&m_state.m_pB[3], m_bOrder);
		m_cState.m_vA2.z = 0.01 * (float)unpack_int16(&m_state.m_pB[5], m_bOrder);

		m_vNangle.x = (float) (m_cState.m_vA2.x - m_vA1range.x) / (float) m_vA1range.len();
		m_vNangle.y = (float) (m_cState.m_vA2.y - m_vA2range.x) / (float) m_vA2range.len();
		m_vNangle.z = (float) (m_cState.m_vA2.z - m_vA3range.x) / (float) m_vA3range.len();
		break;
	case 3:
		m_cState.m_vOrigin.x = 0.1 * (float)unpack_int16(&m_state.m_pB[1], m_bOrder);
		m_cState.m_vOrigin.y = 0.1 * (float)unpack_int16(&m_state.m_pB[3], m_bOrder);
		m_cState.m_vOrigin.z = 0.1 * (float)unpack_int16(&m_state.m_pB[5], m_bOrder);
		break;
	case 4:
		break;
	case 5:
		m_cState.m_vW.x = (float)unpack_int16(&m_state.m_pB[1], m_bOrder);
		m_cState.m_vW.y = (float)unpack_int16(&m_state.m_pB[3], m_bOrder);
		break;
	default:
		break;
	}

}

void _S6H4D::draw(void)
{
	this->_ActuatorBase::draw();

	addMsg("",1);
	addMsg("-- Current state --",1);
	addMsg("vP = (" + f2str(m_cState.m_vP.x) + ", " + f2str(m_cState.m_vP.y) + ", " + f2str(m_cState.m_vP.z) + ")" ,1);
	addMsg("vA1 = (" + f2str(m_cState.m_vA1.x) + ", " + f2str(m_cState.m_vA1.y) + ", " + f2str(m_cState.m_vA1.z) + ")" ,1);
	addMsg("vA2 = (" + f2str(m_cState.m_vA2.x) + ", " + f2str(m_cState.m_vA2.y) + ", " + f2str(m_cState.m_vA2.z) + ")" ,1);
	addMsg("vOrigin = (" + f2str(m_cState.m_vOrigin.x) + ", " + f2str(m_cState.m_vOrigin.y) + ", " + f2str(m_cState.m_vOrigin.z) + ")" ,1);
	addMsg("vW = (" + f2str(m_cState.m_vW.x) + ", " + f2str(m_cState.m_vW.y) + ")" ,1);

	addMsg("",1);
	addMsg("-- Target state --",1);
	addMsg("vP = (" + f2str(m_tState.m_vP.x) + ", " + f2str(m_tState.m_vP.y) + ", " + f2str(m_tState.m_vP.z) + ")" ,1);
	addMsg("vA1 = (" + f2str(m_tState.m_vA1.x) + ", " + f2str(m_tState.m_vA1.y) + ", " + f2str(m_tState.m_vA1.z) + ")" ,1);
	addMsg("vA2 = (" + f2str(m_tState.m_vA2.x) + ", " + f2str(m_tState.m_vA2.y) + ", " + f2str(m_tState.m_vA2.z) + ")" ,1);
	addMsg("vOrigin = (" + f2str(m_tState.m_vOrigin.x) + ", " + f2str(m_tState.m_vOrigin.y) + ", " + f2str(m_tState.m_vOrigin.z) + ")" ,1);
	addMsg("vW = (" + f2str(m_tState.m_vW.x) + ", " + f2str(m_tState.m_vW.y) + ")" ,1);

}

}

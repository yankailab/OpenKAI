#include "_S6H4D.h"

namespace kai
{

_S6H4D::_S6H4D()
{
	m_pIO = NULL;
	m_state.init();

	m_bOrder = true;
	m_mode = 1;
	m_vSpeedRange.init(0.0, 1e5);
	m_speed = 1.0;
	m_vOriginTarget.init(350, 0.0, 400);
	m_vOrigin.init(0.0);
	m_vLastValidP.init(0.0);

	m_vRrange.init(0.0, 500);
	m_nMinAxis = 6;
}

_S6H4D::~_S6H4D()
{
}

bool _S6H4D::init(void *pKiss)
{
	IF_F(!this->_ActuatorBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	pK->v("bOrder", &m_bOrder);
	pK->v("mode", &m_mode);
	pK->v("vSpeedRange", &m_vSpeedRange);
	pK->v("speed", &m_speed);
	pK->v("vOriginTarget", &m_vOriginTarget);
	pK->v("vRrange", &m_vRrange);

	Kiss *pF = pK->child("forbiddenArea");
	int i = 0;
	while (1)
	{
		Kiss *pA = pF->child(i++);
		if (pA->empty())
			break;

		S6H4D_AREA a;
		a.init();
		pA->v("vX", &a.m_vX);
		pA->v("vY", &a.m_vY);
		pA->v("vZ", &a.m_vZ);
		m_vForbArea.push_back(a);
	}

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

	setMode(m_mode);
	while(m_vOrigin != m_vOriginTarget)
	{
		setOrigin(m_vOriginTarget);
		this->sleepTime(USEC_1SEC);
		readState();
	}
	//go to the init pos based on new set origin
	updatePos();

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		readState();
		if(m_vOrigin != m_vOriginTarget)
		{
			setOrigin(m_vOriginTarget);
			continue;
		}

		checkRange();

		updateMove();

		this->autoFPSto();
	}
}

bool _S6H4D::checkRange(void)
{
	IF_F(check() < 0);

	ACTUATOR_AXIS* pX = &m_vAxis[0];
	ACTUATOR_AXIS* pY = &m_vAxis[1];
	ACTUATOR_AXIS* pZ = &m_vAxis[2];
	bool bCheck = true;

	// Radius range
	vFloat2 vP;
	vP.x = pX->m_p.m_vRaw;
	vP.y = pY->m_p.m_vRaw;
	float r = vP.r();
	if(r < m_vRrange.x || r > m_vRrange.y)
//		bCheck = false;

	// Axis range
	for(int i=3; i<6; i++)
	{
		ACTUATOR_AXIS* pA = &m_vAxis[i];
		float v = pA->m_p.m_v;
		IF_CONT(v >= 0.0 && v <= 1.0);

		bCheck = false;
		break;
	}

	// Forbidden area
	vFloat3 vPf;
	vPf.init(pX->m_p.m_vRaw, pY->m_p.m_vRaw, pZ->m_p.m_vRaw);
	for(S6H4D_AREA a : m_vForbArea)
	{
		IF_CONT(a.bInside(vPf));
//		bCheck = false;
		break;
	}

	// Go to the last valid position if the current one is invalid
	if(bCheck)
	{
		m_vLastValidP.x = m_vAxis[0].m_p.m_vRaw;
		m_vLastValidP.y = m_vAxis[1].m_p.m_vRaw;
		m_vLastValidP.z = m_vAxis[2].m_p.m_vRaw;

		return true;
	}

	resetCtrl();
	m_vOrigin.init(0.0);
	while(m_vOrigin != m_vOriginTarget)
	{
		setOrigin(m_vOriginTarget);
		readState();
	}

	vFloat3 vA;
	vA.x = m_vAxis[6].m_p.getTargetRaw();
	vA.y = m_vAxis[7].m_p.getTargetRaw();
	vA.z = m_vAxis[8].m_p.getTargetRaw();
	float s = m_speed * m_vSpeedRange.d() + m_vSpeedRange.x;
	gotoPos(m_vLastValidP, vA, s);

	return bCheck;
}

void _S6H4D::updatePos(void)
{
	IF_(check() < 0);

	vFloat3 vP;
	vP.x = m_vAxis[0].m_p.getTargetRaw();
	vP.y = m_vAxis[1].m_p.getTargetRaw();
	vP.z = m_vAxis[2].m_p.getTargetRaw();

	vFloat3 vA;
	vA.x = m_vAxis[6].m_p.getTargetRaw();
	vA.y = m_vAxis[7].m_p.getTargetRaw();
	vA.z = m_vAxis[8].m_p.getTargetRaw();

	float s = m_speed * m_vSpeedRange.d() + m_vSpeedRange.x;

	gotoPos(vP, vA, s);
}

void _S6H4D::updateMove(void)
{
	IF_(check() < 0);

	vFloat3 vM;
	vM.x = m_vAxis[0].m_s.m_vTarget;
	vM.y = m_vAxis[1].m_s.m_vTarget;
	vM.z = m_vAxis[2].m_s.m_vTarget;
	move(vM);
}

void _S6H4D::updateRot(void)
{
	IF_(check() < 0);

	for(int i=3; i<6; i++)
	{
		ACTUATOR_AXIS* pA = &m_vAxis[i];
		IF_CONT(pA->m_s.m_vTarget < 0.0);

		rot(i, pA->m_s.m_vTarget);
	}
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
	cmd.m_b[4] = (vM.x >= 0.0) ? constrain<float>(128 + (vM.x - 0.5)*255, 0, 255) : 128;
	cmd.m_b[5] = (vM.y >= 0.0) ? constrain<float>(128 + (vM.y - 0.5)*255, 0, 255) : 128;
	cmd.m_b[6] = (vM.z >= 0.0) ? constrain<float>(128 + (vM.z - 0.5)*255, 0, 255) : 128;

	m_pIO->write(cmd.m_b, S6H4D_CMD_N);
}

void _S6H4D::rot(int iAxis, float r)
{
	IF_(check() < 0);
	IF_(iAxis >= m_nMinAxis);

	S6H4D_CMD_CTRL cmd;
	cmd.init();
	cmd.m_b[1] = 30;
	cmd.m_b[2] = 7;
	cmd.m_b[3] = iAxis;
	cmd.m_b[4] = (r >= 0.0) ? constrain<float>(128 + (r - 0.5)*255, 0, 255) : 128;

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

void _S6H4D::resetCtrl(void)
{
	IF_(check() < 0);

	S6H4D_CMD_CTRL cmd;
	cmd.init();
	cmd.m_b[1] = 12;
	cmd.m_b[2] = 6;
	cmd.m_b[3] = 0;

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
		m_vAxis[0].m_p.setRaw(0.1 * (float)unpack_int16(&m_state.m_pB[1], m_bOrder) - m_vOriginTarget.x);
		m_vAxis[1].m_p.setRaw(0.1 * (float)unpack_int16(&m_state.m_pB[3], m_bOrder) - m_vOriginTarget.y);
		m_vAxis[2].m_p.setRaw(0.1 * (float)unpack_int16(&m_state.m_pB[5], m_bOrder) - m_vOriginTarget.z);
		break;
	case 1:
		m_vAxis[3].m_p.setRaw(0.01 * (float)unpack_int16(&m_state.m_pB[1], m_bOrder));
		m_vAxis[4].m_p.setRaw(0.01 * (float)unpack_int16(&m_state.m_pB[3], m_bOrder));
		m_vAxis[5].m_p.setRaw(0.01 * (float)unpack_int16(&m_state.m_pB[5], m_bOrder));
		break;
	case 2:
		m_vAxis[6].m_p.setRaw(0.01 * (float)unpack_int16(&m_state.m_pB[1], m_bOrder));
		m_vAxis[7].m_p.setRaw(0.01 * (float)unpack_int16(&m_state.m_pB[3], m_bOrder));
		m_vAxis[8].m_p.setRaw(0.01 * (float)unpack_int16(&m_state.m_pB[5], m_bOrder));
		break;
	case 3:
		m_vOrigin.x = 0.1 * (float)unpack_int16(&m_state.m_pB[1], m_bOrder);
		m_vOrigin.y = 0.1 * (float)unpack_int16(&m_state.m_pB[3], m_bOrder);
		m_vOrigin.z = 0.1 * (float)unpack_int16(&m_state.m_pB[5], m_bOrder);
		break;
	case 4:
		break;
	case 5:
		vFloat3 vW;
		vW.x = (float)unpack_int16(&m_state.m_pB[1], m_bOrder);
		vW.y = (float)unpack_int16(&m_state.m_pB[3], m_bOrder);
		break;
	default:
		break;
	}
}

void _S6H4D::draw(void)
{
	this->_ActuatorBase::draw();

	addMsg("vOriginTarget = (" + f2str(m_vOriginTarget.x) + ", " + f2str(m_vOriginTarget.y) + ", " + f2str(m_vOriginTarget.z) + ")", 1);
	addMsg("vOrigin = (" + f2str(m_vOrigin.x) + ", " + f2str(m_vOrigin.y) + ", " + f2str(m_vOrigin.z) + ")", 1);

}

}

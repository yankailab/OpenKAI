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
	setOrigin(m_vOriginTarget);

	//go to the origin
	updatePos();

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		readState();

		updateMove();
//		updateRot();

		this->autoFPSto();
	}
}

void _S6H4D::updatePos(void)
{
	IF_(check() < 0);

	vFloat3 vP;
	vP.x = m_vAxis[0].getRawPtarget();
	vP.y = m_vAxis[1].getRawPtarget();
	vP.z = m_vAxis[2].getRawPtarget();

	vFloat3 vA;
	vA.x = m_vAxis[3].getRawPtarget();
	vA.y = m_vAxis[4].getRawPtarget();
	vA.z = m_vAxis[5].getRawPtarget();

	float s = m_speed * m_vSpeedRange.d() + m_vSpeedRange.x;

	gotoPos(vP, vA, s);
}

void _S6H4D::updateMove(void)
{
	IF_(check() < 0);

	vFloat3 vM;
	vM.x = m_vAxis[0].getStarget();
	vM.y = m_vAxis[1].getStarget();
	vM.z = m_vAxis[2].getStarget();
	move(vM);
}

void _S6H4D::updateRot(void)
{
	IF_(check() < 0);

	for(int i=3; i<6; i++)
	{
		ACTUATOR_AXIS* pA = &m_vAxis[i];
		IF_CONT(pA->m_nS < 0.0);

		rot(i, pA->getStarget());
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

	vFloat3 vPlim;
	vPlim.x = constrain(vP.x, m_vAxis[0].m_vPrange.x, m_vAxis[0].m_vPrange.y);
	vPlim.y = constrain(vP.y, m_vAxis[1].m_vPrange.x, m_vAxis[1].m_vPrange.y);
	vPlim.z = constrain(vP.z, m_vAxis[2].m_vPrange.x, m_vAxis[2].m_vPrange.y);

	vFloat3 vRlim;
	vRlim.x = constrain(vR.x, m_vAxis[3].m_vPrange.x, m_vAxis[3].m_vPrange.y);
	vRlim.y = constrain(vR.y, m_vAxis[4].m_vPrange.x, m_vAxis[4].m_vPrange.y);
	vRlim.z = constrain(vR.z, m_vAxis[5].m_vPrange.x, m_vAxis[5].m_vPrange.y);

	S6H4D_CMD_MOV cmd;
	cmd.init('1', 0);
	cmd.set(vPlim, vRlim, speed);
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

	ACTUATOR_AXIS* pA;
	uint8_t* pC;

	pA = &m_vAxis[0];
	pC = &cmd.m_b[4];
	if((pA->m_nP >= 1.0 && *pC > 128) || (pA->m_nP <= 0.0 && *pC < 128))*pC=128;

	pA = &m_vAxis[1];
	pC = &cmd.m_b[5];
	if((pA->m_nP >= 1.0 && *pC > 128) || (pA->m_nP <= 0.0 && *pC < 128))*pC=128;

	pA = &m_vAxis[2];
	pC = &cmd.m_b[6];
	if((pA->m_nP >= 1.0 && *pC < 128) || (pA->m_nP <= 0.0 && *pC > 128))*pC=128;

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
	cmd.m_b[4] = (r >= 0.0 && m_vAxis[iAxis].bNormal()) ? constrain<float>(128 + (r - 0.5)*255, 0, 255) : 128;

	ACTUATOR_AXIS* pA = &m_vAxis[iAxis];
	uint8_t* pC = &cmd.m_b[4];
	if((pA->m_nP >= 1.0 && *pC > 128) || (pA->m_nP <= 0.0 && *pC < 128))*pC=128;

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
		m_vAxis[0].setRawP(0.1 * (float)unpack_int16(&m_state.m_pB[1], m_bOrder) - m_vOriginTarget.x);
		m_vAxis[1].setRawP(0.1 * (float)unpack_int16(&m_state.m_pB[3], m_bOrder) - m_vOriginTarget.y);
		m_vAxis[2].setRawP(0.1 * (float)unpack_int16(&m_state.m_pB[5], m_bOrder) - m_vOriginTarget.z);
		break;
	case 1:
		if(m_vAxis.size() < 9)break;
		m_vAxis[6].setRawP(0.01 * (float)unpack_int16(&m_state.m_pB[1], m_bOrder));
		m_vAxis[7].setRawP(0.01 * (float)unpack_int16(&m_state.m_pB[3], m_bOrder));
		m_vAxis[8].setRawP(0.01 * (float)unpack_int16(&m_state.m_pB[5], m_bOrder));
		break;
	case 2:
		m_vAxis[3].setRawP(0.01 * (float)unpack_int16(&m_state.m_pB[1], m_bOrder));
		m_vAxis[4].setRawP(0.01 * (float)unpack_int16(&m_state.m_pB[3], m_bOrder));
		m_vAxis[5].setRawP(0.01 * (float)unpack_int16(&m_state.m_pB[5], m_bOrder));
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

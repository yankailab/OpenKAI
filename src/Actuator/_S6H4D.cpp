#include "_S6H4D.h"

namespace kai
{

_S6H4D::_S6H4D()
{
	m_pIO = NULL;
	m_state.init();

	m_vPosRangeX.init(0.0, 100.0);
	m_vPosRangeY.init(0.0, 100.0);
	m_vPosRangeZ.init(0.0, 100.0);

	m_vRotRangeX.init(0.0, 180.0);
	m_vRotRangeY.init(0.0, 180.0);
	m_vRotRangeZ.init(0.0, 180.0);

	m_vSpeedRange.init(0.0, 1e5);
}

_S6H4D::~_S6H4D()
{
}

bool _S6H4D::init(void *pKiss)
{
	IF_F(!this->_ActuatorBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	pK->v("vPosRangeX", &m_vPosRangeX);
	pK->v("vPosRangeY", &m_vPosRangeY);
	pK->v("vPosRangeZ", &m_vPosRangeZ);

	pK->v("vRotRangeX", &m_vRotRangeX);
	pK->v("vRotRangeY", &m_vRotRangeY);
	pK->v("vRotRangeZ", &m_vRotRangeZ);

	pK->v("vSpeedRange", &m_vSpeedRange);

	string iName = "";
	F_ERROR_F(pK->v("_IOBase", &iName));
	m_pIO = (_IOBase*) (pK->root()->getChildInst(iName));
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
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updatePos();
		readState();

		this->autoFPSto();
	}
}

void _S6H4D::updatePos(void)
{
	vFloat3 vP;
	vP.x = m_vNormTargetPos.x * m_vPosRangeX.d() + m_vPosRangeX.x;
	vP.y = m_vNormTargetPos.y * m_vPosRangeY.d() + m_vPosRangeY.x;
	vP.z = m_vNormTargetPos.z * m_vPosRangeZ.d() + m_vPosRangeZ.x;

	vFloat3 vR;
	vR.x = m_vNormTargetRot.x * m_vRotRangeX.d() + m_vRotRangeX.x;
	vR.y = m_vNormTargetRot.y * m_vRotRangeY.d() + m_vRotRangeY.x;
	vR.z = m_vNormTargetRot.z * m_vRotRangeZ.d() + m_vRotRangeZ.x;

	float spd = m_vNormTargetSpeed.x * m_vSpeedRange.d() + m_vSpeedRange.x;

	S6H4D_CMD cmd;
	cmd.init();
	cmd.m_cmd1 = '1';
	cmd.m_cmd2 = 1;
	cmd.m_pV[0] = vP.x;
	cmd.m_pV[1] = vP.y;
	cmd.m_pV[2] = vP.z;
	cmd.m_pV[3] = vR.x;
	cmd.m_pV[4] = vR.y;
	cmd.m_pV[5] = vR.z;
	cmd.m_pV[6] = 0;
	cmd.m_pV[7] = 0;
	cmd.m_pV[8] = 0;
	cmd.m_pV[9] = 0;
	cmd.m_pV[10] = spd;
	cmd.m_end = 0xef;

	m_pIO->write((uint8_t*)&cmd, sizeof(S6H4D_CMD));
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
		else if (b == S6H4D_BEGIN)
		{
			m_state.m_pB[0] = b;
			m_state.m_iB = 1;
		}
	}
}

void _S6H4D::decodeState(void)
{
	//	m_vPos.x = pP[0];
	//	m_vPos.y = pP[1];
	//	m_vPos.z = pP[2];
		m_vNormPos.x = (float) (m_vPos.x - m_vPosRangeX.x) / (float) m_vPosRangeX.len();
		m_vNormPos.y = (float) (m_vPos.y - m_vPosRangeY.x) / (float) m_vPosRangeY.len();
		m_vNormPos.z = (float) (m_vPos.z - m_vPosRangeZ.x) / (float) m_vPosRangeZ.len();

	//	m_vRot.x = pP[3];
	//	m_vRot.y = pP[4];
	//	m_vRot.z = pP[5];
		m_vNormRot.x = (float) (m_vRot.x - m_vRotRangeX.x) / (float) m_vRotRangeX.len();
		m_vNormRot.y = (float) (m_vRot.y - m_vRotRangeY.x) / (float) m_vRotRangeY.len();
		m_vNormRot.z = (float) (m_vRot.z - m_vRotRangeZ.x) / (float) m_vRotRangeZ.len();
}

void _S6H4D::draw(void)
{
	this->_ActuatorBase::draw();
}

}

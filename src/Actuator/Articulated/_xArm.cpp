#include "_xArm.h"

namespace kai
{

	_xArm::_xArm()
	{
		m_pArm = NULL;
		m_ip = "192.168.1.222";
		m_mode = 0;
		m_state = 0;
		m_nMinAxis = 6;
	}

	_xArm::~_xArm()
	{
	}

	bool _xArm::init(void *pKiss)
	{
		IF_F(!this->_ActuatorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("ip", &m_ip);
		pK->v("mode", &m_mode);
		pK->v("state", &m_state);

		m_pArm = new XArmAPI(m_ip, false, true);

		return true;
	}

	bool _xArm::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _xArm::check(void)
	{
		IF__(!m_bPower, -1);

		return this->_ActuatorBase::check();
	}

	void _xArm::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			if (!m_bPower)
			{
				if (!power(true))
				{
					m_pT->sleepT(SEC_2_USEC);
					continue;
				}
			}

			readState();

			m_pT->autoFPSto();
		}
	}

	bool _xArm::power(bool bON)
	{
		IF_T(bON == m_bPower);

		if (bON)
		{
			m_pArm->connect();
			m_pArm->motion_enable(true);
			m_pArm->set_mode(0);
			m_pArm->set_state(0);
			m_bPower = true;
		}
		else
		{
			m_bPower = false;
		}
	}

	void _xArm::gotoPos(vFloat3 &vP)
	{
		pthread_mutex_lock(&m_mutex);
		setPtarget(0, vP.x);
		setPtarget(1, vP.y);
		setPtarget(2, vP.z);
		pthread_mutex_unlock(&m_mutex);

		updatePos();
	}

	vFloat3 _xArm::getPtarget(void)
	{
		pthread_mutex_lock(&m_mutex);
		vFloat3 v(m_vAxis[0].m_p.m_vTarget,
				  m_vAxis[1].m_p.m_vTarget,
				  m_vAxis[2].m_p.m_vTarget);
		pthread_mutex_unlock(&m_mutex);

		return v;
	}

	vFloat3 _xArm::getP(void)
	{
		pthread_mutex_lock(&m_mutex);
		vFloat3 v(m_vAxis[0].m_p.m_v,
				  m_vAxis[1].m_p.m_v,
				  m_vAxis[2].m_p.m_v);
		pthread_mutex_unlock(&m_mutex);

		return v;
	}

	vFloat3 _xArm::getAtarget(void)
	{
		pthread_mutex_lock(&m_mutex);
		vFloat3 v(m_vAxis[6].m_p.m_vTarget,
				  m_vAxis[7].m_p.m_vTarget,
				  m_vAxis[8].m_p.m_vTarget);
		pthread_mutex_unlock(&m_mutex);

		return v;
	}

	vFloat3 _xArm::getA(void)
	{
		pthread_mutex_lock(&m_mutex);
		vFloat3 v(m_vAxis[6].m_p.m_v,
				  m_vAxis[7].m_p.m_v,
				  m_vAxis[8].m_p.m_v);
		pthread_mutex_unlock(&m_mutex);

		return v;
	}

	void _xArm::updatePos(void)
	{
		IF_(check() < 0);

		vFloat3 vP = getPtarget();
		vFloat3 vA = getAtarget();
	}

	void _xArm::readState(void)
	{
		IF_(check() < 0);

		int ret;
		fp32 pose[6];
		ret = m_pArm->get_position(pose);

		fp32 angles[7];
		ret = m_pArm->get_servo_angle(angles);
	}

}

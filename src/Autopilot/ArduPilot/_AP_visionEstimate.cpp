#include "_AP_visionEstimate.h"

namespace kai
{

	_AP_visionEstimate::_AP_visionEstimate()
	{
		m_pAP = NULL;
		m_pNav = NULL;

		m_linearAccelCov = 0.01;
		m_angularVelCov = 0.01;
		m_H_aeroRef_T265ref = Matrix4f{{0, 0, -1, 0},
									   {1, 0, 0, 0},
									   {0, -1, 0, 0},
									   {0, 0, 0, 1}};

		m_H_T265body_aeroBody = m_H_aeroRef_T265ref.inverse();

		m_thrJumpPos = 0.1;	 // m
		m_thrJumpSpd = 20.0; // m/s
		m_iReset = 0;

		m_vAxisRPY.set(0,1,2);

		m_bNaN = false;
		m_bPos = true;
		m_bSpd = true;
	}

	_AP_visionEstimate::~_AP_visionEstimate()
	{
	}

	bool _AP_visionEstimate::init(void *pKiss)
	{
		IF_F(!this->_StateBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("bPos", &m_bPos);
		pK->v("bSpd", &m_bSpd);
		pK->v("thrJumpPos", &m_thrJumpPos);
		pK->v("thrJumpSpd", &m_thrJumpSpd);
		pK->v("vAxisRPY", &m_vAxisRPY);

		return true;
	}

	bool _AP_visionEstimate::link(void)
	{
		IF_F(!this->_StateBase::link());

		Kiss *pK = (Kiss *)m_pKiss;

		string n;

		n = "";
		F_ERROR_F(pK->v("_AP_base", &n));
		m_pAP = (_AP_base *)(pK->getInst(n));
		IF_Fl(!m_pAP, n + ": not found");

		n = "";
		F_ERROR_F(pK->v("_NavBase", &n));
		m_pNav = (_NavBase *)(pK->getInst(n));
		IF_Fl(!m_pNav, n + ": not found");

		return true;
	}

	bool _AP_visionEstimate::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _AP_visionEstimate::check(void)
	{
		NULL__(m_pAP, -1);
		NULL__(m_pAP->m_pMav, -1);
		NULL__(m_pNav, -1);

		return this->_StateBase::check();
	}

	void _AP_visionEstimate::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();
			this->_StateBase::update();

			updateVisionEstimate();

			m_pT->autoFPSto();
		}
	}

	void _AP_visionEstimate::updateVisionEstimate(void)
	{
		IF_(check() < 0);

		m_bNaN = bNaN();
		if(m_bNaN)
		{
			LOG_E("Nav returned NaN");

			// Land for emergency?

			return;
		}

		m_conf = m_pNav->confidence();
		m_covPose = m_linearAccelCov * pow(10, 3 - int(m_conf));
		m_covTwist = m_angularVelCov * pow(10, 1 - int(m_conf));
		updateResetCounter();

		sendPosEstimate();
		sendSpeedEstimate();
	}

	bool _AP_visionEstimate::bNaN(void)
	{
		IF_F(check() < 0);

		vFloat3 v = m_pNav->t();
		IF_T(isnan(v.x));
		IF_T(isnan(v.y));
		IF_T(isnan(v.z));
		
		v = m_pNav->v();
		IF_T(isnan(v.x));
		IF_T(isnan(v.y));
		IF_T(isnan(v.z));

		return false;
	}

	void _AP_visionEstimate::updateResetCounter(void)
	{
		vFloat3 vT = m_pNav->t();
		vFloat3 vV = m_pNav->v();

		vFloat3 dvT = vT - m_vTprev;
		vFloat3 dvV = vV - m_vVprev;
		m_vTprev = vT;
		m_vVprev = vV;

		float dT = dvT.len();
		float dV = dvV.len();

		IF_((dT < m_thrJumpPos) && (dV < m_thrJumpSpd));

		m_iReset++;
		if (m_iReset > 255)
			m_iReset = 1;
	}

	void _AP_visionEstimate::sendPosEstimate(void)
	{
		Matrix4f H_T265ref_T265body = m_pNav->mT();
		Matrix4f H_aeroRef_aeroBody = m_H_aeroRef_T265ref * (H_T265ref_T265body * m_H_T265body_aeroBody);
		Matrix3f mRot = H_aeroRef_aeroBody.block(0, 0, 3, 3);
		Vector3f vRPY = mRot.eulerAngles(m_vAxisRPY.x,
										 m_vAxisRPY.y,
										 m_vAxisRPY.z);

		float vCov[21] = {m_covPose, 0, 0, 0, 0, 0,
						  m_covPose, 0, 0, 0, 0,
						  m_covPose, 0, 0, 0,
						  m_covTwist, 0, 0,
						  m_covTwist, 0,
						  m_covTwist};

		m_Dpos.x = H_aeroRef_aeroBody(0, 3);
		m_Dpos.y = H_aeroRef_aeroBody(1, 3);
		m_Dpos.z = H_aeroRef_aeroBody(2, 3);
		m_Dpos.roll = vRPY(0);
		m_Dpos.pitch = vRPY(1);
		m_Dpos.yaw = vRPY(2);
		memcpy(m_Dpos.covariance, vCov, sizeof(float) * 21);
		m_Dpos.reset_counter = m_iReset;
		m_pAP->m_pMav->visionPositionEstimate(m_Dpos);
	}

	void _AP_visionEstimate::sendSpeedEstimate(void)
	{
		vFloat3 vV = m_pNav->v();
		Matrix4f V_aeroRef_aeroBody;
		V_aeroRef_aeroBody.block(0, 0, 3, 3) = Eigen::Quaternionf(1, 0, 0, 0).toRotationMatrix();
		V_aeroRef_aeroBody(0, 3) = vV.x;
		V_aeroRef_aeroBody(1, 3) = vV.y;
		V_aeroRef_aeroBody(2, 3) = vV.z;
		V_aeroRef_aeroBody = m_H_aeroRef_T265ref * V_aeroRef_aeroBody;

		float vCov2[9] = {m_covPose, 0, 0,
						   0, m_covPose, 0,
						   0, 0, m_covPose};

		m_Dspd.x = V_aeroRef_aeroBody(0, 3);
		m_Dspd.y = V_aeroRef_aeroBody(1, 3);
		m_Dspd.z = V_aeroRef_aeroBody(2, 3);
		memcpy(m_Dspd.covariance, vCov2, sizeof(float) * 9);
		m_Dpos.reset_counter = m_iReset;
		m_pAP->m_pMav->visionSpeedEstimate(m_Dspd);
	}

	void _AP_visionEstimate::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_StateBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		string msg;
		msg = "vPos = (" + f2str(m_Dpos.x, 3) + ", " + f2str(m_Dpos.y, 3) + ", " + f2str(m_Dpos.z, 3) + ")";
		pC->addMsg(msg, 1);

		msg = "vYPR = (" + f2str(m_Dpos.roll, 3) + ", " + f2str(m_Dpos.pitch, 3) + ", " + f2str(m_Dpos.yaw, 3) + ")";
		pC->addMsg(msg, 1);

		msg = "vSpd = (" + f2str(m_Dspd.x, 3) + ", " + f2str(m_Dspd.y, 3) + ", " + f2str(m_Dspd.z, 3) + ")";
		pC->addMsg(msg, 1);

		msg = "iReset = " + i2str(m_iReset);
		pC->addMsg(msg, 1);

		msg = "bNaN = " + i2str(m_bNaN);
		pC->addMsg(msg, 1);
	}

}

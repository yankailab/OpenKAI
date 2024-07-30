#include "_AP_visionEstimate.h"

namespace kai
{

	_AP_visionEstimate::_AP_visionEstimate()
	{
		m_pAP = nullptr;
		m_pNav = nullptr;

		m_linearAccelCov = 0.01;
		m_angularVelCov = 0.01;
		m_mTsensor2aero = Matrix4f{{0, 0, -1, 0},
								   {1, 0, 0, 0},
								   {0, -1, 0, 0},
								   {0, 0, 0, 1}}; // default for T265
		m_mTaero2sensor = m_mTsensor2aero.inverse();

		m_thrJumpPos = 0.1;	 // m
		m_thrJumpSpd = 20.0; // m/s
		m_iReset = 0;
		m_vAxisRPY.set(0, 1, 2);

		m_apModeInError = -1;
		m_bNaN = false;
		m_bPos = true;
		m_bSpd = true;
	}

	_AP_visionEstimate::~_AP_visionEstimate()
	{
	}

	int _AP_visionEstimate::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("bPos", &m_bPos);
		pK->v("bSpd", &m_bSpd);
		pK->v("thrJumpPos", &m_thrJumpPos);
		pK->v("thrJumpSpd", &m_thrJumpSpd);
		pK->v("vAxisRPY", &m_vAxisRPY);
		pK->v("apModeInError", &m_apModeInError);

		vector<float> vT;
		pK->a("mTsensor2aero", &vT);
		if (vT.size() == 16)
		{
			m_mTsensor2aero = Matrix4f{{vT[0], vT[1], vT[2], vT[3]},
									   {vT[4], vT[5], vT[6], vT[7]},
									   {vT[8], vT[9], vT[10], vT[11]},
									   {vT[12], vT[13], vT[14], vT[15]}};
			m_mTaero2sensor = m_mTsensor2aero.inverse();
		}

		return OK_OK;
	}

	int _AP_visionEstimate::link(void)
	{
		CHECK_(this->_ModuleBase::link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		n = "";
		pK->v("_AP_base", &n);
		m_pAP = (_AP_base *)(pK->findModule(n));
		NULL__(m_pAP, OK_ERR_NOT_FOUND);

		n = "";
		pK->v("_NavBase", &n);
		m_pNav = (_NavBase *)(pK->findModule(n));
		NULL__(m_pNav, OK_ERR_NOT_FOUND);

		return true;
	}

	int _AP_visionEstimate::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _AP_visionEstimate::check(void)
	{
		NULL__(m_pAP, OK_ERR_NULLPTR);
		NULL__(m_pAP->m_pMav, OK_ERR_NULLPTR);
		NULL__(m_pNav, OK_ERR_NULLPTR);

		return this->_ModuleBase::check();
	}

	void _AP_visionEstimate::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			if (!updateVisionEstimate())
			{
				if (m_apModeInError >= 0)
				{
					int apMode = m_pAP->getMode();
					if ((apMode == AP_COPTER_AUTO) ||
						(apMode == AP_COPTER_LOITER) ||
						(apMode == AP_COPTER_GUIDED))
					{
						m_pAP->setMode(m_apModeInError);
					}
				}
			}

			m_pT->autoFPSto();
		}
	}

	bool _AP_visionEstimate::updateVisionEstimate(void)
	{
		IF_T(check() < 0);

		m_bNaN = bNaN();
		IF_Fl(m_bNaN, "Nav returned NaN");
		IF_F(m_pNav->bError());

		m_conf = m_pNav->confidence();
		m_covPose = m_linearAccelCov * pow(10, 3 - int(m_conf));
		m_covTwist = m_angularVelCov * pow(10, 1 - int(m_conf));
		updateResetCounter();

		if (m_bPos)
			sendPosEstimate();

		if (m_bSpd)
			sendSpeedEstimate();

		return true;
	}

	bool _AP_visionEstimate::bNaN(void)
	{
		IF_F(check() != OK_OK);;

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
		Matrix4f mTsensorPoseSensorRef = m_pNav->mT();
		Matrix4f mTaeroPoseAeroRef = m_mTsensor2aero * (mTsensorPoseSensorRef * m_mTaero2sensor);
		Matrix3f mRot = mTaeroPoseAeroRef.block(0, 0, 3, 3);
		Vector3f vRPY = mRot.eulerAngles(m_vAxisRPY.x,
										 m_vAxisRPY.y,
										 m_vAxisRPY.z);

		float vCov[21] = {m_covPose, 0, 0, 0, 0, 0,
						  m_covPose, 0, 0, 0, 0,
						  m_covPose, 0, 0, 0,
						  m_covTwist, 0, 0,
						  m_covTwist, 0,
						  m_covTwist};

		m_Dpos.x = mTaeroPoseAeroRef(0, 3);
		m_Dpos.y = mTaeroPoseAeroRef(1, 3);
		m_Dpos.z = mTaeroPoseAeroRef(2, 3);
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
		V_aeroRef_aeroBody = m_mTsensor2aero * V_aeroRef_aeroBody;

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
		this->_ModuleBase::console(pConsole);

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

/*
 * _LCalign.cpp
 *
 *  Created on: Mar 6, 2026
 *      Author: yankai
 */

#include "_LCalign.h"

namespace kai
{

	_LCalign::_LCalign()
	{
		m_pPCin = nullptr;
		m_pV = nullptr;
		m_pIMU = nullptr;
		m_fName = "";

		m_vCsize.set(640, 480);
		m_vCf.set(200.0, 200.0);
		m_vCc.set(320, 240);
		pthread_mutex_init(&m_mtxMat, NULL);
	}

	_LCalign::~_LCalign()
	{
		pthread_mutex_destroy(&m_mtxMat);
	}

	bool _LCalign::init(const json &j)
	{
		IF_F(!this->_PCstream::init(j));

		jKv<int>(j, "vCsize", m_vCsize);
		jKv<double>(j, "vCf", m_vCf);
		jKv<double>(j, "vCc", m_vCc);
		jKv(j, "aCdist", m_aCdist);
		jKv(j, "aCr", m_aCr);
		jKv(j, "aCt", m_aCt);

		jKv(j, "fName", m_fName);
		if (!m_fName.empty())
			loadConfig(m_fName);

		updateMatrices();

		return true;
	}

	bool _LCalign::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_PCstream::link(j, pM));
		string n;

		n = "";
		jKv(j, "_PCin", n);
		m_pPCin = (_PCstream *)(pM->findModule(n));
		IF_Le_F(!m_pPCin, "_PCin not found:" + n);

		n = "";
		jKv(j, "_VisionBase", n);
		m_pV = (_VisionBase *)(pM->findModule(n));
		IF_Le_F(!m_pV, "_VisionBase not found:" + n);

		n = "";
		jKv(j, "_IMUbase", n);
		m_pIMU = (_IMUbase *)(pM->findModule(n));

		return true;
	}

	bool _LCalign::loadConfig(const string &fName)
	{
		JsonCfg jCfg;
		IF_F(!jCfg.parseJsonFile(fName));

		const json &j = jK(jCfg.getJson(), "_LCalign");
		IF_F(!j.is_object());

		jKv<int>(j, "vCsize", m_vCsize);
		jKv<double>(j, "vCf", m_vCf);
		jKv<double>(j, "vCc", m_vCc);
		jKv(j, "aCdist", m_aCdist);
		jKv(j, "aCr", m_aCr);
		jKv(j, "aCt", m_aCt);
		jKv(j, "aIr", m_aIr);
		jKv(j, "aIt", m_aIt);

		updateMatrices();

		return true;
	}

	bool _LCalign::saveConfig(const string &fName)
	{
		json j = json::object();
		j["vCsize"] = {m_vCsize.x, m_vCsize.y};
		j["vCf"] = {m_vCf.x, m_vCf.y};
		j["vCc"] = {m_vCc.x, m_vCc.y};
		j["aCdist"] = m_aCdist;
		j["aCr"] = m_aCr;
		j["aCt"] = m_aCt;
		j["aIr"] = m_aIr;
		j["aIt"] = m_aIt;

		json jC;
		jC["_LCalign"] = j;

		JsonCfg jCfg;
		jCfg.setJson(jC);

		return jCfg.saveToFile(fName);
	}

	bool _LCalign::check(void)
	{
		NULL_F(m_pPCin);
		NULL_F(m_pV);

		return this->_PCstream::check();
	}

	bool _LCalign::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _LCalign::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateCalib();
		}
	}

	void _LCalign::updateCalib(void)
	{
		IF_(!check());

		Frame *pF = m_pV->getFrameRGB();
		NULL_(pF);

		Mat mIn = *pF->m();
		IF_(mIn.empty());
		IF_(mIn.channels() != 3)

		m_vCsize.set(mIn.cols, mIn.rows);
		int nPring = m_pPCin->nP();
		int iP = 0;
		GEOMETRY_POINT *pGp;

		while (pGp = m_pPCin->get(iP++))
		{
			NULL_(pGp);

			vFloat3 vP = pGp->m_vP;
			//			vFloat3 vP(-vPin.y, -vPin.z, vPin.x); // mid360 to cam

			vInt2 vPimg;
			IF_CONT(!L2C(m_vCsize, vP, vPimg));

			Vec3b vCol = mIn.at<Vec3b>(vPimg.y, vPimg.x);
			vFloat3 vC(vCol[2], vCol[1], vCol[0]);
			vC *= 1.0 / 255.0;

			add(vP, vC);
		}
	}

	bool _LCalign::L2C(const vInt2 &vSizeImg, const vFloat3 &vPi, vInt2 &vPo)
	{
		IF_F(m_mRot.empty());

		pthread_mutex_lock(&m_mtxMat);

		Mat p = (Mat_<float>(3, 1) << vPi.x, vPi.y, vPi.z);
		Mat mpCam = m_mRot * p + m_mvT;
		double Zc = mpCam.at<float>(2, 0);
		if (Zc <= 1e-9)
		{
			pthread_mutex_unlock(&m_mtxMat);
			return false;
		}

		// Project
		vector<Point3f> vPlidar = {Point3f(vPi.x, vPi.y, vPi.z)};
		vector<Point2f> vPcam;
		projectPoints(vPlidar, m_mvR, m_mvT, m_mCam, m_mDistCoeffs, vPcam);

		pthread_mutex_unlock(&m_mtxMat);

		IF_F(vPcam.empty());

		Point2f pCam = vPcam[0];
		int px = round(pCam.x);
		int py = round(pCam.y);
		IF_F(px < 0 || px >= vSizeImg.x);
		IF_F(py < 0 || py >= vSizeImg.y);

		vPo.set(px, py);
		return true;
	}

	void _LCalign::updateMatrices(void)
	{
		pthread_mutex_lock(&m_mtxMat);

		// Camera matrix K
		m_mCam = (Mat_<float>(3, 3) << m_vCf.x, 0.0, m_vCc.x,
				  0.0, m_vCf.y, m_vCc.y,
				  0.0, 0.0, 1.0);

		// Distortion coefficients: [k1, k2, p1, p2, k3]
		m_mDistCoeffs = (Mat_<float>(1, 5) << m_aCdist[0],
						 m_aCdist[1],
						 m_aCdist[2],
						 m_aCdist[3],
						 m_aCdist[4]);

		// Rotation matrix (row-major)
		m_mRot = (Mat_<float>(3, 3) << m_aCr[0], m_aCr[1], m_aCr[2],
				  m_aCr[3], m_aCr[4], m_aCr[5],
				  m_aCr[6], m_aCr[7], m_aCr[8]);

		// Convert rotation matrix to Rodrigues vector
		Rodrigues(m_mRot, m_mvR);

		// Translation vector
		m_mvT = (Mat_<float>(3, 1) << m_aCt[0],
				 m_aCt[1],
				 m_aCt[2]);

		pthread_mutex_unlock(&m_mtxMat);
	}

	vDouble2 _LCalign::getCamFocal(void)
	{
		return m_vCf;
	}

	vDouble2 _LCalign::getCamCenter(void)
	{
		return m_vCc;
	}

	array<double, 5> _LCalign::getCamDistortion(void)
	{
		return m_aCdist;
	}

	array<double, 9> _LCalign::getCamR(void)
	{
		return m_aCr;
	}

	array<double, 3> _LCalign::getCamT(void)
	{
		return m_aCt;
	}

	void _LCalign::setCamFocal(const vDouble2 &vF)
	{
		m_vCf = vF;
	}

	void _LCalign::setCamCenter(const vDouble2 &vC)
	{
		m_vCc = vC;
	}

	void _LCalign::setCamDistortion(const array<double, 5> &aD)
	{
		m_aCdist = aD;
	}

	void _LCalign::setCamR(const array<double, 9> &aR)
	{
		m_aCr = aR;
	}

	void _LCalign::getCamT(const array<double, 3> &aT)
	{
		m_aCt = aT;
	}

	void _LCalign::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_PCstream::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		string msg;
		pC->addMsg(msg);
	}

	void _LCalign::console(const json &j, void *pJSONbase)
	{
		_JSONbase *pJb = (_JSONbase *)pJSONbase;

		string cmd;
		IF_(!jKv(j, "cmd", cmd));

		if (cmd == "setParams")
		{
			jKv<int>(j, "vCsize", m_vCsize);
			jKv<double>(j, "vCf", m_vCf);
			jKv<double>(j, "vCc", m_vCc);
			jKv(j, "aCdist", m_aCdist);
			jKv(j, "aCt", m_aCt);

			vFloat3 vCr;
			if (jKv<float>(j, "vCr", vCr))
			{
				// TODO
			}

			updateMatrices();
		}
		else if (cmd == "update")
		{
			NULL_(pJb);

			json jr = json::object();
			jr["cmd"] = "update";
			jr["vCsize"] = {m_vCsize.x, m_vCsize.y};
			jr["vCf"] = {m_vCf.x, m_vCf.y};
			jr["vCc"] = {m_vCc.x, m_vCc.y};
			jr["aCdist"] = m_aCdist;
			jr["aCt"] = m_aCt;

			// vFloat3 vCr;
			// jr["vCr"] = {vCr, };

			pJb->sendJson(jr);
		}
		else if (cmd == "loadCfg")
		{
			string fCfg;
			IF_(!jKv(j, "fNameCfg", fCfg));

			bool bR = loadConfig(fCfg);

			NULL_(pJb);
			json jr = json::object();
			if (bR)
			{
				jr["cmd"] = "update";
				jr["vCsize"] = {m_vCsize.x, m_vCsize.y};
				jr["vCf"] = {m_vCf.x, m_vCf.y};
				jr["vCc"] = {m_vCc.x, m_vCc.y};
				jr["aCdist"] = m_aCdist;
				jr["aCt"] = m_aCt;
			}
			else
			{
				jr["cmd"] = "loadCfg";
				jr["bSuccess"] = bR;
			}

			pJb->sendJson(jr);
		}
		else if (cmd == "saveCfg")
		{
			string fCfg;
			IF_(!jKv(j, "fNameCfg", fCfg));

			bool bR = saveConfig(fCfg);

			NULL_(pJb);
			json jr = json::object();
			jr["cmd"] = "saveCfg";
			jr["bSuccess"] = bR;
			pJb->sendJson(jr);
		}
		else if (cmd == "savePly")
		{
			string fPly;
			IF_(!jKv(j, "fNamePly", fPly));

			bool bR = saveFile(fPly);

			NULL_(pJb);
			json jr = json::object();
			jr["cmd"] = "savePly";
			jr["bSuccess"] = bR;
			pJb->sendJson(jr);
		}
	}

}

// bool _LCalign::pt2Pix(const vInt2 &vSizeImg, const vFloat3 &vPi, vInt2 &vPo)
// {
// 	// --- 1) LiDAR -> Camera ---
// 	const auto &R = m_aCr;
// 	const auto &T = m_aCt;
// 	const vDouble3 vPc(
// 		R[0] * vPi.x + R[1] * vPi.y + R[2] * vPi.z + T[0],
// 		R[3] * vPi.x + R[4] * vPi.y + R[5] * vPi.z + T[1],
// 		R[6] * vPi.x + R[7] * vPi.y + R[8] * vPi.z + T[2]);

// 	// Point must be in front of camera
// 	IF_F(vPc.z <= 1e-9);

// 	// --- 2) Normalize ---
// 	const vDouble2 vPnorm(
// 		vPc.x / vPc.z,
// 		vPc.y / vPc.z);

// 	// --- 3) Distortion ---
// 	const double k1 = m_aCdist[0];
// 	const double k2 = m_aCdist[1];
// 	const double p1 = m_aCdist[2];
// 	const double p2 = m_aCdist[3];
// 	const double k3 = m_aCdist[4];
// 	const double r2 = vPnorm.x * vPnorm.x + vPnorm.y * vPnorm.y;
// 	const double r4 = r2 * r2;
// 	const double r6 = r4 * r2;
// 	const double radial = 1.0 + k1 * r2 + k2 * r4 + k3 * r6;

// 	const double xDist = vPnorm.x * radial + 2.0 * p1 * vPnorm.x * vPnorm.y + p2 * (r2 + 2.0 * vPnorm.x * vPnorm.x);
// 	const double yDist = vPnorm.y * radial + p1 * (r2 + 2.0 * vPnorm.y * vPnorm.y) + 2.0 * p2 * vPnorm.x * vPnorm.y;

// 	// --- 4) Project to pixel ---
// 	const double u = m_vCf.x * xDist + m_vCc.x;
// 	const double v = m_vCf.y * yDist + m_vCc.y;

// 	int px = round(u);
// 	int py = round(v);
// 	IF_F(px < 0 || px >= vSizeImg.x);
// 	IF_F(py < 0 || py >= vSizeImg.y);

// 	vPo.set(px, py);

// 	return true;
// }

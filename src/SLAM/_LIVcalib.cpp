/*
 * _LIVcalib.cpp
 *
 *  Created on: Mar 6, 2026
 *      Author: yankai
 */

#include "_LIVcalib.h"

namespace kai
{

	_LIVcalib::_LIVcalib()
	{
		m_pPCin = nullptr;
		m_pV = nullptr;
		m_pIMU = nullptr;
		m_fName = "";

		m_vCsize.set(640, 480);
		m_vCf.set(400.0, 400.0);
		m_vCc.set(320, 240);
	}

	_LIVcalib::~_LIVcalib()
	{
	}

	bool _LIVcalib::init(const json &j)
	{
		IF_F(!this->_PCstream::init(j));

		jKv(j, "fName", m_fName);

		if(!m_fName.empty())
			loadConfig(m_fName);

		return true;
	}

	bool _LIVcalib::link(const json &j, ModuleMgr *pM)
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

	bool _LIVcalib::loadConfig(const string &fName)
	{
		JsonCfg jCfg;
		IF_F(!jCfg.parseJsonFile(fName));

		const json &j = jK(jCfg.getJson(), "_LIVcalib");
		IF_F(!j.is_object());

		jKv<int>(j, "vCsize", m_vCsize);
		jKv<double>(j, "vCf", m_vCf);
		jKv<double>(j, "vCc", m_vCc);
		jKv(j, "aCdistortion", m_aCdistortion);
		jKv(j, "aCr", m_aCr);
		jKv(j, "aCt", m_aCt);
		jKv(j, "aIr", m_aIr);
		jKv(j, "aIt", m_aIt);

		return true;
	}

	bool _LIVcalib::saveConfig(const string &fName)
	{
		json j;
		j["vCsize"] = {m_vCsize.x, m_vCsize.y};
		j["vCf"] = {m_vCf.x, m_vCf.y};
		j["vCc"] = {m_vCc.x, m_vCc.y};
		j["aCdistortion"] = m_aCdistortion;
		j["aCr"] = m_aCr;
		j["aCt"] = m_aCt;
		j["aIr"] = m_aIr;
		j["aIt"] = m_aIt;

		json jC;
		jC["_LIVcalib"] = j;

		JsonCfg jCfg;
		jCfg.setJson(jC);

		return jCfg.saveToFile(fName);
	}

	bool _LIVcalib::check(void)
	{
		NULL_F(m_pPCin);
		NULL_F(m_pV);

		return this->_PCstream::check();
	}

	bool _LIVcalib::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _LIVcalib::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateCalib();
		}
	}

	void _LIVcalib::updateCalib(void)
	{
		IF_(!check());

		Frame *pF = m_pV->getFrameRGB();
		NULL_(pF);

		Mat mIn = *pF->m();
		IF_(mIn.empty());

		Mat mImg;
		if (mIn.channels() == 3)
			mImg = mIn;
		else if (mIn.channels() == 1)
			cvtColor(mIn, mImg, cv::COLOR_GRAY2BGR);

		m_vCsize.set(mImg.cols, mImg.rows);
		const float cNorm = 1.0 / 255.0;
		int nPring = m_pPCin->nP();
		int iP = 0;
		GEOMETRY_POINT *pGp;

		while (pGp = m_pPCin->get(iP++))
		{
			NULL_(pGp);

			vFloat3 vP = pGp->m_vP;
			vInt2 vPimg;
			IF_CONT(!pt2Pix(m_vCsize, vP, vPimg));

			Vec3b vCol = mImg.at<Vec3b>(vPimg.y, vPimg.x);
			vFloat3 vC(vCol[2], vCol[1], vCol[0]);
			vC *= cNorm;

			add(vP, vC);
		}
	}

	bool _LIVcalib::pt2Pix(const vInt2 &vSizeImg, const vFloat3 &vPi, vInt2 &vPo)
	{
		// --- 1) LiDAR -> Camera ---
		const auto &R = m_aCr;
		const auto &T = m_aCt;
		const vDouble3 vPc(
			R[0] * vPi.x + R[1] * vPi.y + R[2] * vPi.z + T[0],
			R[3] * vPi.x + R[4] * vPi.y + R[5] * vPi.z + T[1],
			R[6] * vPi.x + R[7] * vPi.y + R[8] * vPi.z + T[2]);

		// Point must be in front of camera
		IF_F(vPc.z <= 1e-9);

		// --- 2) Normalize ---
		const vDouble2 vPnorm(
			vPc.x / vPc.z,
			vPc.y / vPc.z);

		// --- 3) Distortion ---
		const double k1 = m_aCdistortion[0];
		const double k2 = m_aCdistortion[1];
		const double p1 = m_aCdistortion[2];
		const double p2 = m_aCdistortion[3];
		const double k3 = m_aCdistortion[4];
		const double r2 = vPnorm.x * vPnorm.x + vPnorm.y * vPnorm.y;
		const double r4 = r2 * r2;
		const double r6 = r4 * r2;
		const double radial = 1.0 + k1 * r2 + k2 * r4 + k3 * r6;

		const double xDist = vPnorm.x * radial + 2.0 * p1 * vPnorm.x * vPnorm.y + p2 * (r2 + 2.0 * vPnorm.x * vPnorm.x);
		const double yDist = vPnorm.y * radial + p1 * (r2 + 2.0 * vPnorm.y * vPnorm.y) + 2.0 * p2 * vPnorm.x * vPnorm.y;

		// --- 4) Project to pixel ---
		const double u = m_vCf.x * xDist + m_vCc.x;
		const double v = m_vCf.y * yDist + m_vCc.y;

		IF_F(u < 0.0 || u >= vSizeImg.x);
		IF_F(v < 0.0 || v >= vSizeImg.y);

		vPo.set(round(u), round(v));

		return true;
	}

	void _LIVcalib::updateMatrices(void)
	{
	}

	vDouble2 _LIVcalib::getCamFocal(void)
	{
		return m_vCf;
	}

	vDouble2 _LIVcalib::getCamCenter(void)
	{
		return m_vCc;
	}

	array<double, 5> _LIVcalib::getCamDistortion(void)
	{
		return m_aCdistortion;
	}

	array<double, 9> _LIVcalib::getCamR(void)
	{
		return m_aCr;
	}

	array<double, 3> _LIVcalib::getCamT(void)
	{
		return m_aCt;
	}

	void _LIVcalib::setCamFocal(const vDouble2 &vF)
	{
		m_vCf = vF;
	}

	void _LIVcalib::setCamCenter(const vDouble2 &vC)
	{
		m_vCc = vC;
	}

	void _LIVcalib::setCamDistortion(const array<double, 5> &aD)
	{
		m_aCdistortion = aD;
	}

	void _LIVcalib::setCamR(const array<double, 9> &aR)
	{
		m_aCr = aR;
	}

	void _LIVcalib::getCamT(const array<double, 3> &aT)
	{
		m_aCt = aT;
	}

	void _LIVcalib::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_PCstream::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		string msg;
		pC->addMsg(msg);
	}
}

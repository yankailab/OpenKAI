/*
 * _ARmeasure.cpp
 *
 *  Created on: July 26, 2021
 *      Author: yankai
 */

#include "_ARmeasure.h"

namespace kai
{

	_ARmeasure::_ARmeasure()
	{
		m_pR = NULL;
		m_pD = NULL;
		m_pN = NULL;
		m_pW = NULL;

		m_fCalibOfs = "";
		m_dirSave = "";
		m_minPoseConfidence = 0.5;
		m_d = 0.0;
		m_bValidDist = false;
		m_bValidPose = false;
		m_vKlaserSpot.init(0.7 / 100.0, -0.2 / 100.0);
		m_vRange.init(0.5, 330.0);
		m_vDofsP.init(0);
		m_vDptW = {0, 0, 0};
		m_vCofsP.init(0);
		m_vAxisIdx.init(0, 1, 2);

		m_vCircleSize.init(10, 20);
		m_crossSize = 20;
		m_dMsg = 100000;
		m_drawCol = Scalar(0, 255, 0);
		m_pFt = NULL;
	}

	_ARmeasure::~_ARmeasure()
	{
	}

	bool _ARmeasure::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("minPoseConfidence", &m_minPoseConfidence);
		pK->v("vRange", &m_vRange);
		pK->v("vKlaserSpot", &m_vKlaserSpot);
		pK->v("vAxisIdx", &m_vAxisIdx);
		pK->v("vCircleSize", &m_vCircleSize);
		pK->v("crossSize", &m_crossSize);
		pK->v("dMsg", &m_dMsg);
		pK->v("vDorgP", &m_vDofsP);
		pK->v("vCorgP", &m_vCofsP);
		pK->v("dirSave", &m_dirSave);

		string n;

		n = "";
		pK->v("_Remap", &n);
		m_pR = (_Remap *)(pK->findModule(n));
		IF_Fl(!m_pR, n + " not found");

		n = "";
		pK->v("_DistSensorBase", &n);
		m_pD = (_DistSensorBase *)(pK->findModule(n));
		IF_Fl(!m_pD, n + " not found");

		n = "";
		pK->v("_NavBase", &n);
		m_pN = (_NavBase *)(pK->findModule(n));
		IF_Fl(!m_pN, n + " not found");

		n = "";
		pK->v("_WindowCV", &n);
		m_pW = (_WindowCV *)(pK->findModule(n));
		IF_Fl(!m_pW, n + " not found");

		m_pW->setCbBtn("Save", sOnBtnSave, this);

		// read offset calib from file if exists
		pK->v("fCalibOfs", &m_fCalibOfs);
		Kiss *pKf = new Kiss();
		if (parseKiss(m_fCalibOfs, pKf))
		{
			pK = pKf->child("calib");
			pK->v("vDofsP", &m_vDofsP);
			pK->v("vCofsP", &m_vCofsP);
		}
		DEL(pKf);

		return true;
	}

	bool _ARmeasure::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _ARmeasure::check(void)
	{
		NULL__(m_pR, -1);
		IF__(m_pR->BGR()->bEmpty(), -1);
		NULL__(m_pD, -1);
		NULL__(m_pN, -1);
		NULL__(m_pW, -1);

		return this->_ModuleBase::check();
	}

	void _ARmeasure::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			this->_ModuleBase::update();
			updateSensor();

			m_pT->autoFPSto();
		}
	}

	bool _ARmeasure::updateSensor(void)
	{
		IF_F(check() < 0);

		if (!m_pD->bReady())
		{
			m_bValidDist = false;
		}
		else
		{
			m_d = m_vDofsP.y + m_pD->d((int)0);
			m_bValidDist = m_vRange.bInside(m_d);
		}

		if (!m_pN->bReady())
			m_bValidPose = false;
		else
			m_bValidPose = (m_pN->confidence() > m_minPoseConfidence);

		IF_d_F(!m_bValidDist, m_msg.set("Distance error", 1, false, m_dMsg));
		IF_d_F(!m_bValidPose, m_msg.set("Tracking lost", 1, false, m_dMsg));

		m_vDptP = {m_vDofsP.x, m_d, m_vDofsP.z};

		Matrix4f mTpose = m_pN->mT();
		m_aPose = mTpose;
		m_vDptW = m_aPose * m_vDptP;

		Matrix3f mRRpose = m_pN->mR().transpose();
		Matrix4f mTwc = Matrix4f::Identity();
		mTwc.block(0, 0, 3, 3) = mRRpose;
		Vector3f mRT = {mTpose(0, 3), mTpose(1, 3), mTpose(2, 3)};
		mRT = mRRpose * mRT;
		mTwc(0, 3) = -mRT(0) - m_vCofsP.x;
		mTwc(1, 3) = -mRT(1) - m_vCofsP.y;
		mTwc(2, 3) = -mRT(2) - m_vCofsP.z;
		m_aW2C = mTwc;

		return true;
	}

	bool _ARmeasure::c2scr(const Vector3f &vPc,
						   const cv::Size &vSize,
						   const vFloat2 &vF,
						   const vFloat2 &vC,
						   cv::Point *pvPs)
	{
		NULL_F(pvPs);

		Vector3f vP = Vector3f(
			vPc[m_vAxisIdx.x],
			-vPc[m_vAxisIdx.y],
			vPc[m_vAxisIdx.z]);

		float ovZ = 1.0 / abs(vP.z());
		pvPs->x = vF.x * vP.x() * ovZ + vC.x;
		pvPs->y = vF.y * vP.y() * ovZ + vC.y;

		return (vP.z() > 0.0);
	}

	bool _ARmeasure::bInsideScr(const cv::Size &s, const cv::Point &p)
	{
		IF_F(p.x < 0);
		IF_F(p.x > s.width);
		IF_F(p.y < 0);
		IF_F(p.y > s.height);

		return true;
	}

	bool _ARmeasure::bValid(void)
	{
		return m_bValidDist & m_bValidPose;
	}

	Vector3f _ARmeasure::vDptW(void)
	{
		return m_vDptW;
	}

	Eigen::Affine3f _ARmeasure::aW2C(void)
	{
		return m_aW2C;
	}

	vFloat3 _ARmeasure::getDofsP(void)
	{
		return m_vDofsP;
	}

	vFloat3 _ARmeasure::getCofsP(void)
	{
		return m_vCofsP;
	}

	void _ARmeasure::setDofsP(const vFloat3 &v)
	{
		m_vDofsP = v;
	}

	void _ARmeasure::setCofsP(const vFloat3 &v)
	{
		m_vCofsP = v;
	}

	// config
	bool _ARmeasure::saveCalib(void)
	{
		picojson::object o;
		o.insert(make_pair("name", "calib"));

		picojson::array v;

		v.push_back(value(m_vDofsP.x));
		v.push_back(value(m_vDofsP.y));
		v.push_back(value(m_vDofsP.z));
		o.insert(make_pair("vDofsP", value(v)));
		v.clear();

		v.push_back(value(m_vCofsP.x));
		v.push_back(value(m_vCofsP.y));
		v.push_back(value(m_vCofsP.z));
		o.insert(make_pair("vCofsP", value(v)));
		v.clear();

		string f = picojson::value(o).serialize();

		_File *pF = new _File();
		IF_F(!pF->open(&m_fCalibOfs, ios::out));
		pF->write((uint8_t *)f.c_str(), f.length());
		pF->close();
		DEL(pF);

		m_msg.set("Calibration saved");

		return true;
	}

	// UI handler
	void _ARmeasure::save(void)
	{
		IF_(check() < 0);

		// save screenshot to USB memory
		Mat m;
		m_pW->getFrame()->m()->copyTo(m);
		vector<int> vPNG;
		vPNG.push_back(IMWRITE_PNG_COMPRESSION);
		vPNG.push_back(1);

		DIR *pDirIn = opendir(m_dirSave.c_str());
		if (!pDirIn)
		{
			m_msg.set(m_dirSave + ": not found", 1);
			return;
		}

		struct dirent *dir;
		string d = "";
		while ((dir = readdir(pDirIn)) != NULL)
		{
			IF_CONT(dir->d_name[0] == '.');
			IF_CONT(dir->d_type != 0x4); //0x4: folder

			d = m_dirSave + string(dir->d_name);
			d = checkDirName(d);
			break;
		}

		closedir(pDirIn);

		if (d.empty())
		{
			m_msg.set("Insert USB memory", 1);
			return;
		}

		if (imwrite(d + tFormat() + ".png", m, vPNG))
		{
			m_msg.set("Saved to USB memory");
		}
		else
		{
			m_msg.set("Cannot write to USB memory", 1);
		}
	}

	// callbacks
	void _ARmeasure::sOnBtnSave(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasure *)pInst)->save();
	}

	// UI draw
	void _ARmeasure::setMsg(const string &msg, int type, bool bOverride)
	{
		m_msg.set(msg, type, bOverride);
	}

	void _ARmeasure::drawCross(Mat *pM)
	{
		NULL_(pM);
		IF_(!m_bValidDist);

		vFloat2 vF = m_pR->getFf();
		vFloat2 vC = m_pR->getCf();
		cv::Size s = m_pR->BGR()->size();

		// laser spot
		float yD = m_vDofsP.y + m_d;
		Vector3f vLSc = {m_vDofsP.x,
						 yD,
						 m_vDofsP.z};
		Vector3f vLSl = {m_vDofsP.x + (yD * m_vKlaserSpot.y),
						 yD,
						 m_vDofsP.z};
		Vector3f vLSt = {m_vDofsP.x,
						 yD,
						 m_vDofsP.z + (yD * m_vKlaserSpot.x)};

		Eigen::Affine3f aL2C = m_aW2C * m_aPose;
		vLSc = aL2C * vLSc;
		vLSl = aL2C * vLSl;
		vLSt = aL2C * vLSt;

		cv::Point vPc, vPl, vPt;
		c2scr(vLSc, s, vF, vC, &vPc);
		c2scr(vLSl, s, vF, vC, &vPl);
		c2scr(vLSt, s, vF, vC, &vPt);

		float rd = (m_vRange.y - m_d) / m_vRange.len();
		Scalar col = (m_bValidDist) ? Scalar(0, 255.0 * rd, 255.0 * (1.0 - rd)) : Scalar(0, 0, 255);

		Rect2i r;
		r.x = vPl.x;
		r.y = vPt.y;
		r.width = abs(vPc.x - vPl.x) * 2 + 1;
		r.height = abs(vPc.y - vPt.y) * 2 + 1;
		rectangle(*pM, r, col, 1);

		// target cross
		line(*pM,
			 Point(vPc.x - m_crossSize, vPc.y),
			 Point(vPc.x + m_crossSize, vPc.y),
			 col,
			 1);

		line(*pM,
			 Point(vPc.x, vPc.y - m_crossSize),
			 Point(vPc.x, vPc.y + m_crossSize),
			 col,
			 1);
	}

	void _ARmeasure::drawLidarRead(Mat *pM)
	{
		NULL_(pM);
		NULL_(m_pFt);

		Scalar col = (m_bValidDist) ? Scalar(0, 255, 0) : Scalar(0, 0, 255);
		string sD;
		if (m_bValidDist)
			sD = f2str(m_d, 2) + "m";
		else
			sD = "Err";

		Point pt;
		pt.x = 10;
		pt.y = pM->rows - 45;

		m_pFt->putText(*pM, sD,
					   pt,
					   40,
					   col,
					   -1,
					   cv::LINE_AA,
					   false);
	}

	void _ARmeasure::drawMsg(Mat *pM)
	{
		NULL_(pM);
		NULL_(m_pFt);

		IF_(!m_msg.update());

		int baseline = 0;
		Size ts = m_pFt->getTextSize(m_msg.get(),
									 40,
									 -1,
									 &baseline);

		Point pt;
		pt.x = constrain(320 - ts.width / 2, 0, pM->cols);
		pt.y = constrain(pM->rows / 2 - ts.height, 0, pM->rows);
		Scalar c = Scalar(0, 255, 0);
		if (m_msg.m_type == 1)
			c = Scalar(0, 0, 255);

		m_pFt->putText(*pM, m_msg.get(),
					   pt,
					   40,
					   c,
					   -1,
					   cv::LINE_AA,
					   false);
	}

	void _ARmeasure::draw(void *pFrame)
	{
		NULL_(pFrame);
		this->_ModuleBase::draw(pFrame);
		IF_(check() < 0);

		Frame *pF = (Frame*)pFrame;
		Mat *pMw = pF->m();
		IF_(pMw->empty());
		m_pFt = pWin->getFont();

		drawCross(pMw);
		drawLidarRead(pMw);
		drawMsg(pMw);
	}

	void _ARmeasure::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
	}

}

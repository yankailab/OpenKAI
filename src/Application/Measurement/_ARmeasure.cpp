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
		m_pV = NULL;
		m_pD = NULL;
		m_pN = NULL;
		m_pW = NULL;

		m_mode = ARmeasure_area;
		m_minPoseConfidence = 0.5;
		m_d = 0.0;
		m_bValidDist = false;
		m_bValidPose = false;
		m_vKlaserSpot.init(0.7 / 100.0, -0.2 / 100.0);
		m_vRange.init(0.5, 330.0);
		m_vDorgP.init(0);
		m_vDptW = {0, 0, 0};
		m_vCorgP.init(0);
		m_vAxisIdx.init(0, 1, 2);

		m_vCircleSize.init(10, 20);
		m_crossSize = 20;
		m_drawCol = Scalar(0, 255, 0);
		m_drawMsg = "";
		m_pFt = NULL;

		m_pTs = NULL;
		m_cmdBatt = "python3 INA219.py";
		m_battV = 0;
		m_battA = 0;
		m_battW = 0;
		m_battP = 100;
		m_battShutdown = 10;
	}

	_ARmeasure::~_ARmeasure()
	{
		DEL(m_pTs);
	}

	bool _ARmeasure::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("minPoseConfidence", &m_minPoseConfidence);
		pK->v("vRange", &m_vRange);
		pK->v("vKlaserSpot", &m_vKlaserSpot);
		pK->v("vAxisIdx", &m_vAxisIdx);
		pK->v("vDorgP", &m_vDorgP);
		pK->v("vCorgP", &m_vCorgP);

		pK->v("vCircleSize", &m_vCircleSize);
		pK->v("crossSize", &m_crossSize);

		pK->v("cmdBatt", &m_cmdBatt);
		pK->v("battShutdown", &m_battShutdown);

		string n;

		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->getInst(n));
		IF_Fl(!m_pV, n + " not found");

		n = "";
		pK->v("_DistSensorBase", &n);
		m_pD = (_DistSensorBase *)(pK->getInst(n));
		IF_Fl(!m_pD, n + " not found");

		n = "";
		pK->v("_NavBase", &n);
		m_pN = (_NavBase *)(pK->getInst(n));
		IF_Fl(!m_pN, n + " not found");

		n = "";
		pK->v("_WindowCV", &n);
		m_pW = (_WindowCV *)(pK->getInst(n));
		IF_Fl(!m_pW, n + " not found");

		m_pW->setCbBtn("Add", sOnBtnAction, this);
		m_pW->setCbBtn("Save", sOnBtnSave, this);
		m_pW->setCbBtn("Clear", sOnBtnClear, this);
		m_pW->setCbBtn("Mode", sOnBtnMode, this);

		// slow jobs
		Kiss *pKt = pK->child("threadSlow");
		IF_F(pKt->empty());
		m_pTs = new _Thread();
		if (!m_pTs->init(pKt))
		{
			DEL(m_pTs);
			return false;
		}

		return true;
	}

	bool _ARmeasure::start(void)
	{
		NULL_F(m_pT);
		NULL_F(m_pTs);
		IF_F(!m_pT->start(getUpdate, this));
		return m_pTs->start(getUpdateSlow, this);
	}

	int _ARmeasure::check(void)
	{
		NULL__(m_pV, -1);
		IF__(m_pV->BGR()->bEmpty(), -1);
		NULL__(m_pD, -1);
		NULL__(m_pN, -1);
		NULL__(m_pW, -1);

		return 0;
	}

	void _ARmeasure::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			if (updatePose())
			{
			}

			m_pT->autoFPSto();
		}
	}

	bool _ARmeasure::updatePose(void)
	{
		IF_F(check() < 0);

		if (!m_pD->bReady())
		{
			m_bValidDist = false;
		}
		else
		{
			m_d = m_pD->d((int)0);
			m_bValidDist = m_vRange.bInside(m_d);
		}

		if (!m_pN->bReady())
			m_bValidPose = false;
		else
			m_bValidPose = (m_pN->confidence() > m_minPoseConfidence);

		IF_F(!m_bValidDist);
		IF_F(!m_bValidPose);


		m_vDptP = {m_vDorgP.x, m_vDorgP.y + m_d, m_vDorgP.z};

		Matrix4f mTpose = m_pN->mT();
		m_aPose = mTpose;
		m_vDptW = m_aPose * m_vDptP;

		Matrix3f mRRpose = m_pN->mR().transpose();
		Matrix4f mTwc = Matrix4f::Identity();
		mTwc.block(0, 0, 3, 3) = mRRpose;
		Vector3f mRT = {mTpose(0, 3), mTpose(1, 3), mTpose(2, 3)};
		mRT = mRRpose * mRT;
		mTwc(0, 3) = -mRT(0); // - m_vCorgP.x;
		mTwc(1, 3) = -mRT(1); // - m_vCorgP.y;
		mTwc(2, 3) = -mRT(2); // - m_vCorgP.z;
		m_aW2C = mTwc;

		return true;
	}

	void _ARmeasure::updateArea(void)
	{
		int nV = m_vVert.size();
		Vector3f vA(0, 0, 0);
		int j = 0;

		for (int i = 0; i < nV; i++)
		{
			j = (i + 1) % nV;
			vA += m_vVert[i].m_vVertW.cross(m_vVert[j].m_vVertW);
		}

		vA *= 0.5;
		m_result = vA.norm();
	}

	void _ARmeasure::updateDist(void)
	{

	}

	void _ARmeasure::updateFreeArea(void)
	{

	}

	void _ARmeasure::updateFreeDist(void)
	{

	}

	// UI handler
	void _ARmeasure::action(void)
	{
		IF_(!m_bValidDist);

		ARAREA_VERTEX av;
		av.m_vVertW = m_vDptW;
		m_vVert.push_back(av);
	}

	void _ARmeasure::save(void)
	{
	}

	void _ARmeasure::clear(void)
	{
	}

	void _ARmeasure::mode(void)
	{
		int i = (int)m_mode + 1;
		m_mode = (ARmeasure_Mode)(i % 4);

		m_pW->setBtnLabel("Mode",ARmeasureModeLabel[(int)m_mode]);
	}

	// callbacks
	void _ARmeasure::sOnBtnAction(void *pInst)
	{
		NULL_(pInst);
		((_ARmeasure *)pInst)->action();
	}

	void _ARmeasure::sOnBtnSave(void *pInst)
	{
		NULL_(pInst);
		((_ARmeasure *)pInst)->save();
	}

	void _ARmeasure::sOnBtnClear(void *pInst)
	{
		NULL_(pInst);
		((_ARmeasure *)pInst)->clear();
	}

	void _ARmeasure::sOnBtnMode(void *pInst)
	{
		NULL_(pInst);
		((_ARmeasure *)pInst)->mode();
	}

	// slow update jobs
	void _ARmeasure::updateSlow(void)
	{
		while (m_pTs->bRun())
		{
			m_pTs->autoFPSfrom();

			updateBatt();

			m_pTs->autoFPSto();
		}
	}

	bool _ARmeasure::updateBatt(void)
	{
		// check battery
		FILE *pFr = popen(m_cmdBatt.c_str(), "r");
		IF_F(pFr <= 0);

		char pB[256];
		float pBatt[4]; //voltage, current, power, percent
		int i;
		for (i = 0; i < 4 && fgets(pB, sizeof(pB), pFr) != NULL; i++)
			pBatt[i] = atof(pB);

		IF_F(i < 4);

		m_battV = pBatt[0];
		m_battA = pBatt[1];
		m_battW = pBatt[2];
		m_battP = pBatt[3];
		pclose(pFr);

		LOG_I("Battery: V=" + f2str(m_battV) +
			  ", A=" + f2str(m_battA) +
			  ", W=" + f2str(m_battW) +
			  ", P=" + f2str(m_battP));

		if (m_battP < m_battShutdown)
			system("shutdown -P now");

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

	// UI draw
	void _ARmeasure::drawCross(Mat *pM)
	{
		NULL_(pM);

		vFloat2 vF = m_pV->getFf();
		vFloat2 vC = m_pV->getCf();
		cv::Size s = m_pV->BGR()->size();

		// laser spot
		Vector3f vLSc = {m_vDorgP.x, m_vDorgP.y + m_d, m_vDorgP.z};
		Vector3f vLSl = {m_vDorgP.x + m_d * m_vKlaserSpot.y,
						 m_vDorgP.y + m_d,
						 m_vDorgP.z};
		Vector3f vLSt = {m_vDorgP.x,
						 m_vDorgP.y + m_d,
						 m_vDorgP.z + m_d * m_vKlaserSpot.x};

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

		if (m_bValidDist)
		{
			Rect2i r;
			r.x = vPl.x;
			r.y = vPt.y;
			r.width = abs(vPc.x - vPl.x) * 2 + 1;
			r.height = abs(vPc.y - vPt.y) * 2 + 1;
			rectangle(*pM, r, col, 1);
		}

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

	void _ARmeasure::drawVertices(Mat *pM)
	{
		NULL_(pM);
		NULL_(m_pFt);

		vFloat2 vF = m_pV->getFf();
		vFloat2 vC = m_pV->getCf();
		cv::Size s = m_pV->BGR()->size();

		// vertices
		int i, j;
		int nV = m_vVert.size();
		for (i = 0; i < nV; i++)
		{
			ARAREA_VERTEX *pA = &m_vVert[i];
			Vector3f vPc = m_aW2C * pA->m_vVertW;
			pA->m_bZ = c2scr(vPc, s, vF, vC, &pA->m_vPs);

			IF_CONT(!pA->m_bZ);
			IF_CONT(!bInsideScr(s, pA->m_vPs));

			circle(*pM, pA->m_vPs, 10, m_drawCol, 3, cv::LINE_AA);
		}

		IF_(nV <= 1);

		// distances between vertices
		for (i = 0; i < nV; i++)
		{
			j = (i + 1) % nV;
			ARAREA_VERTEX *pA = &m_vVert[i];
			ARAREA_VERTEX *pB = &m_vVert[j];
			IF_CONT(!pA->m_bZ && !pB->m_bZ);

			Point p = pA->m_vPs;
			Point q = pB->m_vPs;
			IF_CONT(!clipLine(s, p, q));

			line(*pM, p, q, m_drawCol, 3, cv::LINE_AA);
			Vector3f vD = pA->m_vVertW - pB->m_vVertW;

			string sL = f2str(vD.norm(), 2) + "m";
			m_pFt->putText(*pM, sL,
						   (p + q) * 0.5,
						   20,
						   m_drawCol,
						   -1,
						   cv::LINE_AA,
						   false);
		}
	}

	void _ARmeasure::drawArea(Mat *pM)
	{
		NULL_(pM);
		NULL_(m_pFt);

		int nV = m_vVert.size();
		IF_(nV <= 2);

		// area
		string sA = "Area = " + f2str(m_result, 2);
		int baseline = 0;
		Size ts = m_pFt->getTextSize(sA,
									 40,
									 -1,
									 &baseline);

		Point pt;
		pt.x = 200;
		pt.y = pM->rows - 45;

		m_pFt->putText(*pM, sA,
					   pt,
					   40,
					   Scalar(255, 255, 255),
					   -1,
					   cv::LINE_AA,
					   false);

		pt.x += ts.width + 10;
		m_pFt->putText(*pM, "m",
					   pt,
					   40,
					   Scalar(255, 255, 255),
					   -1,
					   cv::LINE_AA,
					   false);

		pt.x += 25;
		m_pFt->putText(*pM, "2",
					   pt,
					   25,
					   Scalar(255, 255, 255),
					   -1,
					   cv::LINE_AA,
					   false);
	}

	void _ARmeasure::drawDist(Mat *pM)
	{
		NULL_(pM);
	}

	void _ARmeasure::drawLidarRead(Mat *pM)
	{
		NULL_(pM);
		NULL_(m_pFt);

		Rect r;
		r.x = 0;
		r.y = 440;
		r.width = 640;
		r.height = 40;
		(*pM)(r) = Scalar(0);

		Scalar col = (m_bValidDist) ? Scalar(255, 255, 255) : Scalar(0, 0, 255);
		string sD = "D = ";
		if (m_bValidDist)
			sD += f2str(m_d, 2) + "m";
		else
			sD += "Err";

		Point pt;
		pt.x = 20;
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

		m_drawMsg = "";

		if (m_pN->confidence() < m_minPoseConfidence)
			m_drawMsg = "Tracking lost";

		IF_(m_drawMsg.empty());

		int baseline = 0;
		Size ts = m_pFt->getTextSize(m_drawMsg,
									 40,
									 -1,
									 &baseline);

		Point pt;
		pt.x = constrain(320 - ts.width / 2, 0, pM->cols);
		pt.y = constrain(pM->rows / 2 - ts.height, 0, pM->rows);

		m_pFt->putText(*pM, m_drawMsg,
					   pt,
					   40,
					   Scalar(0, 0, 255),
					   -1,
					   cv::LINE_AA,
					   false);
	}

	void _ARmeasure::drawBatt(Mat *pM)
	{
		NULL_(pM);
		NULL_(m_pFt);

		Scalar col = (m_battP > m_battShutdown * 1.5) ? Scalar(255, 255, 255) : Scalar(0, 0, 255);
		string sB = "Bat." + i2str((int)m_battP) + "%";

		Point pt;
		pt.x = 500;
		pt.y = pM->rows - 45;

		m_pFt->putText(*pM, sB,
					   pt,
					   40,
					   col,
					   -1,
					   cv::LINE_AA,
					   false);
	}

	void _ARmeasure::cvDraw(void *pWindow)
	{
		NULL_(pWindow);
		this->_ModuleBase::cvDraw(pWindow);
		IF_(check() < 0);

		_WindowCV *pWin = (_WindowCV *)pWindow;
		Frame *pF = pWin->getFrame();
		NULL_(pF);
		Mat *pMw = pF->m();
		IF_(pMw->empty());
		m_pFt = pWin->getFont();

		// video input
		Mat mV;
		m_pV->BGR()->m()->copyTo(mV);

		drawCross(&mV);
		drawVertices(&mV);

		Rect r;
		r.x = 0;
		r.y = 0;
		r.width = mV.cols;
		r.height = mV.rows;
		mV.copyTo((*pMw)(r));

		drawLidarRead(pMw);
		drawArea(pMw);
		drawBatt(pMw);
		drawMsg(pMw);
	}

	void _ARmeasure::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		NULL_(m_pTs);
		m_pTs->console(pConsole);
	}

}

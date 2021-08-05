/*
 * _ARarea.cpp
 *
 *  Created on: July 26, 2021
 *      Author: yankai
 */

#include "_ARarea.h"

#ifdef USE_OPENCV

namespace kai
{

	_ARarea::_ARarea()
	{
		m_pV = NULL;
		m_pD = NULL;
		m_pN = NULL;

		m_vRange.init(0.0, 100.0);
		m_vDorgP.init(0);
		m_vDptW = {0, 0, 0};
		m_vCorgP.init(0);
		m_vAxisIdx.init(0, 1, 2);

		m_pTs = NULL;
		m_cmdBatt = "python3 INA219.py";
		m_battShutdown = 10;
	}

	_ARarea::~_ARarea()
	{
		DEL(m_pTs);
	}

	bool _ARarea::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vRange", &m_vRange);
		pK->v("vAxisIdx", &m_vAxisIdx);
		pK->v("vDorgP", &m_vDorgP);
		pK->v("vCorgP", &m_vCorgP);
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
		_WindowCV *pW = (_WindowCV *)(pK->getInst(n));
		IF_Fl(!pW, n + " not found");

		pW->setCbBtn("Add", sOnBtnAdd, this);
		pW->setCbBtn("Clear", sOnBtnClear, this);
		pW->setCbBtn("Save", sOnBtnSave, this);

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

	bool _ARarea::start(void)
	{
		NULL_F(m_pT);
		NULL_F(m_pTs);
		IF_F(!m_pT->start(getUpdate, this));
		return m_pTs->start(getUpdateSlow, this);
	}

	int _ARarea::check(void)
	{
		NULL__(m_pV, -1);
		IF__(m_pV->BGR()->bEmpty(), -1);
		NULL__(m_pD, -1);
		NULL__(m_pN, -1);

		return 0;
	}

	void _ARarea::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			updateARarea();

			m_pT->autoFPSto();
		}
	}

	bool _ARarea::updateARarea(void)
	{
		IF_F(check() < 0);

		m_d = m_pD->d((int)0);
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

		return false;
	}

	bool _ARarea::c2scr(const Vector3f &vPc,
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

	bool _ARarea::bInsideScr(const cv::Size &s, const cv::Point &p)
	{
		IF_F(p.x < 0);
		IF_F(p.x > s.width);
		IF_F(p.y < 0);
		IF_F(p.y > s.height);

		return true;
	}

	float _ARarea::area(void)
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
		return vA.norm();
	}

	void _ARarea::cvDraw(void *pWindow)
	{
		NULL_(pWindow);
		this->_ModuleBase::cvDraw(pWindow);
		IF_(check() < 0);

		_WindowCV *pWin = (_WindowCV *)pWindow;
		Frame *pF = pWin->getFrame();
		NULL_(pF);
		Mat *pMw = pF->m();
		IF_(pMw->empty());
		cv::Ptr<freetype::FreeType2> pFt = pWin->getFont();

		Mat mV;
		m_pV->BGR()->m()->copyTo(mV);

		vFloat2 vF = m_pV->getFf();
		vFloat2 vC = m_pV->getCf();
		cv::Size s = m_pV->BGR()->size();
		Scalar col = Scalar(0, 255, 0);

		Vector3f vDptC = m_aW2C * m_vDptW;
		cv::Point vPs;
		c2scr(vDptC, s, vF, vC, &vPs);
		circle(mV, vPs, 10, col, 3, cv::LINE_AA);

		int i, j;
		int nV = m_vVert.size();

		for (i = 0; i < nV; i++)
		{
			ARAREA_VERTEX *pA = &m_vVert[i];
			Vector3f vPc = m_aW2C * pA->m_vVertW;
			pA->m_bZ = c2scr(vPc, s, vF, vC, &pA->m_vPs);

			IF_CONT(!pA->m_bZ);
			IF_CONT(!bInsideScr(s, pA->m_vPs));

			circle(mV, pA->m_vPs, 10, col, 3, cv::LINE_AA);
		}

		if (nV > 1)
		{
			for (i = 0; i < nV; i++)
			{
				j = (i + 1) % nV;
				ARAREA_VERTEX *pA = &m_vVert[i];
				ARAREA_VERTEX *pB = &m_vVert[j];
				IF_CONT(!pA->m_bZ && !pB->m_bZ);

				Point p = pA->m_vPs;
				Point q = pB->m_vPs;
				IF_CONT(!clipLine(s, p, q));

				line(mV, p, q, col, 3, cv::LINE_AA);
				Vector3f vD = pA->m_vVertW - pB->m_vVertW;

				string sL = f2str(vD.norm(), 2) + "m";
				if (pFt)
				{
					pFt->putText(mV, sL,
								 (p + q) * 0.5,
								 20,
								 col,
								 -1,
								 cv::LINE_AA,
								 false);
				}
				else
				{
					putText(mV, f2str(vD.norm(), 2) + "m",
							(p + q) * 0.5,
							FONT_HERSHEY_SIMPLEX, 0.6, col, 1, cv::LINE_AA);
				}
			}
		}

		Rect r;
		r.x = 0;
		r.y = 0;
		r.width = mV.cols;
		r.height = mV.rows;
		mV.copyTo((*pMw)(r));

		if (nV > 2)
		{
			r.x = 0;
			r.y = 440;
			r.width = 640;
			r.height = 40;
			(*pMw)(r) = Scalar(0);

			string sA = "Area = " + f2str(area(), 2) + " Sq. M";
			if (pFt)
			{
				pFt->putText(*pMw, sA,
							 Point(40, pMw->rows - 40),
							 40,
							 Scalar(255, 255, 255),
							 -1,
							 cv::LINE_AA,
							 false);
			}
			else
			{
				putText(*pMw, sA,
						Point(10, pMw->rows - 20),
						FONT_HERSHEY_SIMPLEX, 0.8, col, 1, cv::LINE_AA);
			}
		}
	}

	void _ARarea::addVertex(void)
	{
		ARAREA_VERTEX av;
		av.m_vVertW = m_vDptW;
		m_vVert.push_back(av);
	}

	void _ARarea::sOnBtnAdd(void *pInst)
	{
		NULL_(pInst);
		_ARarea *pA = (_ARarea *)pInst;
		pA->addVertex();
	}

	void _ARarea::sOnBtnClear(void *pInst)
	{
		NULL_(pInst);
		_ARarea *pA = (_ARarea *)pInst;
		pA->m_vVert.clear();
	}

	void _ARarea::sOnBtnSave(void *pInst)
	{
		NULL_(pInst);
		_ARarea *pA = (_ARarea *)pInst;
	}

	void _ARarea::updateSlow(void)
	{
		while (m_pTs->bRun())
		{
			m_pTs->autoFPSfrom();

			updateBatt();

			m_pTs->autoFPSto();
		}
	}

	bool _ARarea::updateBatt(void)
	{
		// check battery
		FILE *pFr = popen(m_cmdBatt.c_str(), "r");
		IF_F(pFr <= 0);

		char pB[256];
		float pBatt[4]; //voltage, current, power, percent
		for (int i = 0; i < 4 && fgets(pB, sizeof(pB), pFr) != NULL; i++)
			pBatt[i] = atof(pB);

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

	void _ARarea::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		NULL_(m_pTs);
		m_pTs->console(pConsole);
	}

}
#endif

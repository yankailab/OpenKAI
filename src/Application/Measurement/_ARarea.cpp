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
	}

	_ARarea::~_ARarea()
	{
	}

	bool _ARarea::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vRange", &m_vRange);
		pK->v("vAxisIdx", &m_vAxisIdx);
		pK->v("vDorgP", &m_vDorgP);
		pK->v("vCorgP", &m_vCorgP);

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

		return true;
	}

	bool _ARarea::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
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

		Matrix4f mTwc = Matrix4f::Identity();
		mTwc.block(0, 0, 3, 3) = m_pN->mR().transpose();
		mTwc(0, 3) = -mTpose(0, 3) - m_vCorgP.x;
		mTwc(1, 3) = -mTpose(1, 3) - m_vCorgP.y;
		mTwc(2, 3) = -mTpose(2, 3) - m_vCorgP.z;
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

		bool b = true; //wether inside the projection plane
		if (vP.z() < 0.0)
			b = false;
		if (pvPs->x < 0 || pvPs->x > vSize.width)
			b = false;
		if (pvPs->y < 0 || pvPs->y > vSize.height)
			b = false;

		return b;
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
		Mat *pM = pF->m();
		IF_(pM->empty());

		Mat mV;
		m_pV->BGR()->m()->copyTo(mV);
		vFloat2 vF = m_pV->getFf();
		vFloat2 vC = m_pV->getCf();
		cv::Size s = mV.size();
		Scalar col = Scalar(0, 255, 0);

		Vector3f vDptC = m_aW2C * m_vDptW;
		cv::Point vPs;
		if (c2scr(vDptC, s, vF, vC, &vPs))
		{
			circle(mV, vPs, 10, col, 3);
		}

		int i, j;
		int nV = m_vVert.size();

		for (i = 0; i < nV; i++)
		{
			ARAREA_VERTEX *pA = &m_vVert[i];
			Vector3f vPc = m_aW2C * pA->m_vVertW;
			pA->m_bP = c2scr(vPc, s, vF, vC, &pA->m_vPs);

			IF_CONT(!pA->m_bP);
			circle(mV, pA->m_vPs, 10, col, 3);
		}

		if (nV > 1)
		{
			for (i = 0; i < nV; i++)
			{
				j = (i + 1) % nV;
				ARAREA_VERTEX *pA = &m_vVert[i];
				ARAREA_VERTEX *pB = &m_vVert[j];
				IF_CONT(!pA->m_bP && !pB->m_bP);

				Point p = pA->m_vPs;
				Point q = pB->m_vPs;
				clipLine(s, p, q);
				line(mV, p, q, col, 3);

				Vector3f vD = pA->m_vVertW - pB->m_vVertW;
				putText(mV, f2str(vD.norm(), 2) + "m",
						(p + q) * 0.5,
						FONT_HERSHEY_SIMPLEX, 0.6, col, 1);
			}
		}

		Mat m;
		cv::resize(mV, m, pM->size());
		m.copyTo(*pM);

		if (nV > 2)
		{
			putText(*pM, f2str(area(), 2) + " m^2",
					Point(0, pM->rows - 10),
					FONT_HERSHEY_SIMPLEX, 0.8, col, 1);
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

}
#endif

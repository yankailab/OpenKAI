/*
 * _ARmeasureVertex.cpp
 *
 *  Created on: July 26, 2021
 *      Author: yankai
 */

#include "_ARmeasureVertex.h"

namespace kai
{

	_ARmeasureVertex::_ARmeasureVertex()
	{
		m_pA = NULL;
		m_pR = NULL;
		m_pW = NULL;

		m_area = 0;
		m_Dtot = 0;

		m_drawCol = Scalar(0, 255, 0);
		m_pFt = NULL;
	}

	_ARmeasureVertex::~_ARmeasureVertex()
	{
	}

	bool _ARmeasureVertex::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		string n;

		n = "";
		pK->v("_ARmeasure", &n);
		m_pA = (_ARmeasure *)(pK->findModule(n));
		IF_Fl(!m_pA, n + " not found");

		n = "";
		pK->v("_Remap", &n);
		m_pR = (_Remap *)(pK->findModule(n));
		IF_Fl(!m_pR, n + " not found");

		n = "";
		pK->v("_WindowCV", &n);
		m_pW = (_WindowCV *)(pK->findModule(n));
		IF_Fl(!m_pW, n + " not found");

		return true;
	}

	bool _ARmeasureVertex::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _ARmeasureVertex::check(void)
	{
		NULL__(m_pA, -1);
		NULL__(m_pW, -1);
		NULL__(m_pR, -1);
		IF__(m_pR->BGR()->bEmpty(), -1);

		return 0;
	}

	void _ARmeasureVertex::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			this->_ModuleBase::update();
			updateVertex();

			m_pT->autoFPSto();
		}
	}

	void _ARmeasureVertex::updateVertex(void)
	{
		IF_(!bActive());
		if (bStateChanged())
		{
			m_pW->setCbBtn("Action", sOnBtnAction, this);
			m_pW->setCbBtn("Clear", sOnBtnClear, this);
			m_pW->setCbBtn("Mode", sOnBtnMode, this);

			m_pW->setBtnLabel("Mode", "V");

			clear();
		}

		m_area = calcArea();
	}

	float _ARmeasureVertex::calcArea(void)
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

	// callbacks
	void _ARmeasureVertex::sOnBtnAction(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasureVertex *)pInst)->action();
	}

	void _ARmeasureVertex::sOnBtnClear(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasureVertex *)pInst)->clear();
	}

	void _ARmeasureVertex::sOnBtnMode(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasureVertex *)pInst)->mode(f);
	}

	// UI handler
	void _ARmeasureVertex::action(void)
	{
		IF_(!m_pA->bValid());

		// add new vertex
		ARMEASURE_VERTEX av;
		av.m_vVertW = m_pA->vDptW();
		m_vVert.push_back(av);
	}

	void _ARmeasureVertex::clear(void)
	{
		m_vVert.clear();
	}

	void _ARmeasureVertex::mode(uint32_t f)
	{
		NULL_(m_pSC);

		if (f & 1) //long push
			m_pSC->transit("calibCam");
		else
			m_pSC->transit("free");
	}

	// UI draw
	void _ARmeasureVertex::drawVertices(Mat *pM)
	{
		NULL_(pM);
		NULL_(m_pFt);

		vFloat2 vF = m_pR->getFf();
		vFloat2 vC = m_pR->getCf();
		cv::Size s = m_pR->BGR()->size();

		// vertices
		int i, j;
		int nV = m_vVert.size();
		for (i = 0; i < nV; i++)
		{
			ARMEASURE_VERTEX *pA = &m_vVert[i];
			Vector3f vPc = m_pA->aW2C() * pA->m_vVertW;
			pA->m_bZ = m_pA->c2scr(vPc, s, vF, vC, &pA->m_vPs);

			IF_CONT(!pA->m_bZ);
			IF_CONT(!m_pA->bInsideScr(s, pA->m_vPs));

			circle(*pM, pA->m_vPs, 3, m_drawCol, 1, cv::LINE_AA);
		}

		IF_(nV <= 1);

		// distances between vertices
		for (i = 0; i < nV; i++)
		{
			j = (i + 1) % nV;
			ARMEASURE_VERTEX *pA = &m_vVert[i];
			ARMEASURE_VERTEX *pB = &m_vVert[j];
			IF_CONT(!pA->m_bZ && !pB->m_bZ);

			Point p = pA->m_vPs;
			Point q = pB->m_vPs;
			IF_CONT(!clipLine(s, p, q));

			line(*pM, p, q, m_drawCol, 1, cv::LINE_AA);
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

	void _ARmeasureVertex::drawMeasure(Mat *pM)
	{
		NULL_(pM);
		NULL_(m_pFt);

		int nV = m_vVert.size();
		IF_(nV <= 2);

		// area
		string sA = "Area = " + f2str(m_area, 2);
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

	void _ARmeasureVertex::draw(void* pFrame)
	{
		NULL_(pFrame);
		IF_(!bActive());

		this->_ModuleBase::draw(pFrame);
		IF_(check() < 0);

		Frame *pF = (Frame*)pFrame;
		Mat *pMw = pF->m();
		IF_(pMw->empty());
		m_pFt = pWin->getFont();

		// video input
		Mat mV;
		m_pR->BGR()->m()->copyTo(mV);

		drawVertices(&mV);

		Rect r;
		r.x = 0;
		r.y = 0;
		r.width = mV.cols;
		r.height = mV.rows;
		mV.copyTo((*pMw)(r));

		r.x = 0;
		r.y = 440;
		r.width = 640;
		r.height = 40;
		(*pMw)(r) = Scalar(0);

		drawMeasure(pMw);
	}

}

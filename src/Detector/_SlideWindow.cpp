/*
 * _SlideWindow.cpp
 *
 *  Created on: June 2, 2019
 *      Author: yankai
 */

#include "_SlideWindow.h"

#ifdef USE_OPENCV

namespace kai
{

	_SlideWindow::_SlideWindow()
	{
		//	m_pC = NULL;
		m_pD = NULL;
		m_vRoi.init();
		m_vRoi.z = 1.0;
		m_vRoi.w = 1.0;
		m_w = 0.2;
		m_dW = 0.1;
		m_nW = 0;
		m_maxD = 0.6;
		m_dMinArea = 0.3;
		m_dRange.init();
	}

	_SlideWindow::~_SlideWindow()
	{
	}

	bool _SlideWindow::init(void *pKiss)
	{
		IF_F(!this->_DetectorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vRoi", &m_vRoi);
		pK->v("w", &m_w);
		pK->v("dW", &m_dW);
		pK->v("maxD", &m_maxD);
		pK->v("dRange", &m_dRange);
		pK->v("dMinArea", &m_dMinArea);

		string n = "";
		//	F_ERROR_F(pK->v("_DNNclassifier", &n));
		//	m_pC = (_DNNclassifier*) (pK->getInst(n));
		//	NULL_Fl(m_pC, n + " not found");

		F_ERROR_F(pK->v("_DepthVisionBase", &n));
		m_pD = (_DepthVisionBase *)(pK->getInst(n));
		NULL_Fl(m_pD, n + " not found");

		m_nW = 0;
		while (m_vRoi.x + m_nW * m_dW * m_vRoi.width() + m_w * m_vRoi.width() < m_vRoi.z)
			m_nW++;
		if (m_nW <= 0)
		{
			LOG_E("nW <=0");
			return false;
		}

		m_nClass = 4; //m_pC->m_nClass;

		return true;
	}

	bool _SlideWindow::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _SlideWindow::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			if (check() >= 0)
			{
				detect();

				if (m_pT->bGoSleep())
					m_pU->clear();
			}

			m_pT->autoFPSto();
		}
	}

	int _SlideWindow::check(void)
	{
		NULL__(m_pU, -1);
		NULL__(m_pV, -1);
		IF__(m_pV->BGR()->bEmpty(), -1);
		NULL__(m_pD, -1);
		IF__(m_pD->BGR()->bEmpty(), -1);

		return this->_DetectorBase::check();
	}

	void _SlideWindow::detect(void)
	{
		m_pV->BGR()->m()->copyTo(m_mBGR);
		m_pD->BGR()->m()->copyTo(m_mD);
		cv::inRange(m_mD, 1, m_maxD * 255.0, m_mDin);

		vInt2 cs = m_pD->getSize();

		_Object o;
		float rW = m_vRoi.width();
		for (int i = 0; i < m_nW; i++)
		{
			vFloat4 bb;
			bb.x = i * m_dW * rW + m_vRoi.x;
			bb.z = bb.x + m_w * rW;
			bb.y = m_vRoi.y;
			bb.w = m_vRoi.w;

			o.init();
			o.setBB2D(bb);

			Rect r = bb2Rect(o.getBB2Dscaled(m_mDin.cols, m_mDin.rows));
			Mat mDinR = m_mDin(r);
			int nP = cv::countNonZero(mDinR);
			IF_CONT((float)nP / (float)r.area() < m_dMinArea);

			//		o.m_dist = (hist(m_mD(rbb), 0, 255, m_nLevel, m_minArea)/255.0) * m_dRange.len() + m_dRange.x;
			o.setZ(((float)cv::mean(m_mD(r), mDinR).val[0] / 255.0) * m_dRange.len() + m_dRange.x);

			m_pU->add(o);
			//		if(m_pC->classify(m_mBGR(r),&o))
			//		{
			//			add(&o);
			//		}
		}

		m_pU->swap();
	}

	void _SlideWindow::draw(void* pFrame)
	{
		NULL_(pFrame);
		this->_DetectorBase::draw(pFrame);
		IF_(check() < 0);

		_WindowCV *pWin = (_WindowCV *)pFrame;
		Frame *pF = pWin->getNextFrame();
		NULL_(pF);
		Mat *pM = pF->m();
		IF_(pM->empty());

		Rect r = bb2Rect(bbScale(m_vRoi, pM->cols, pM->rows));
		rectangle(*pM, r, Scalar(0, 255, 0), 1);

		IF_(!m_bDebug);

		if (!m_mBGR.empty())
			imshow(*this->getName() + ":BGR", m_mBGR);

		if (!m_mD.empty())
			imshow(*this->getName() + ":Depth", m_mD);

		if (!m_mDin.empty())
			imshow(*this->getName() + ":InRange", m_mDin);
	}

}
#endif

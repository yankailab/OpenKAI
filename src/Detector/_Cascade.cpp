/*
 *  Created on: Sept 28, 2016
 *      Author: yankai
 */
#include "_Cascade.h"

namespace kai
{

	_Cascade::_Cascade()
	{
		m_scaleFactor = 1.1;
		m_minNeighbors = 3;
		m_className = "";
		m_bGPU = true;
	}

	_Cascade::~_Cascade()
	{
	}

	int _Cascade::init(void *pKiss)
	{
		CHECK_(this->_DetectorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("scaleFactor", &m_scaleFactor);
		pK->v("minNeighbors", &m_minNeighbors);
		pK->v("bGPU", &m_bGPU);

		if (m_bGPU)
		{
			m_pGCC = cuda::CascadeClassifier::create(m_fModel);
			NULL__(m_pGCC, OK_ERR_NOT_FOUND);
		}
		else
		{
			IF__(!m_CC.load(m_fModel), OK_ERR_NOT_FOUND);
		}

		return OK_OK;
	}

	int _Cascade::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _Cascade::check(void)
	{
		NULL__(m_pV, OK_ERR_NULLPTR);
		NULL__(m_pU, OK_ERR_NULLPTR);

		return this->_DetectorBase::check();
	}

	void _Cascade::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			if (m_bGPU)
				detectGPU();
			else
				detectCPU();

			ON_PAUSE;
		}
	}

	void _Cascade::detectGPU(void)
	{
		IF_(check() != OK_OK);

		Frame *pGray = m_pV->getFrameRGB();
		NULL_(pGray);
		IF_(pGray->bEmpty());
		GpuMat m;
		pGray->gm()->copyTo(m);

//		int minSize = m.cols * m_pU->m_rArea.x;
//		int maxSize = m.cols * m_pU->m_rArea.y;

		m_pGCC->setFindLargestObject(false);
		m_pGCC->setScaleFactor(m_scaleFactor);
		m_pGCC->setMinNeighbors(m_minNeighbors);
//		m_pGCC->setMinObjectSize(cv::Size(minSize, minSize));
//		m_pGCC->setMaxObjectSize(cv::Size(maxSize, maxSize));

		vector<Rect> vRect;
		cv::cuda::GpuMat gFound;
		m_pGCC->detectMultiScale(m, gFound);
		m_pGCC->convert(gFound, vRect);

		_Object o;
		float kx = 1.0 / m.cols;
		float ky = 1.0 / m.rows;

		for (int i = 0; i < vRect.size(); i++)
		{
			o.setBB2D(rect2BB<vFloat4>(vRect[i]));
			o.scale(kx, ky);
			o.setTopClass(0, 1.0);
			o.setZ(0.0);
			o.setText(m_className);
			o.setTstamp(m_pT->getTfrom());

			m_pU->add(o);
		}

		m_pU->swap();
	}

	void _Cascade::detectCPU(void)
	{
		IF_(check() != OK_OK);

		Frame *pGray = m_pV->getFrameRGB();
		NULL_(pGray);
		IF_(pGray->bEmpty());
		Mat m;
		pGray->m()->copyTo(m);

//		int minSize = m.cols * m_pU->m_rArea.x;
//		int maxSize = m.cols * m_pU->m_rArea.y;

		vector<Rect> vRect;
		_Object o;
		float kx = 1.0 / m.cols;
		float ky = 1.0 / m.rows;

		// m_CC.detectMultiScale(m, vRect, m_scaleFactor, m_minNeighbors,
		// 					  0 | CASCADE_SCALE_IMAGE, Size(minSize, minSize),
		// 					  Size(maxSize, maxSize));

		for (int i = 0; i < vRect.size(); i++)
		{
			o.setBB2D(rect2BB<vFloat4>(vRect[i]));
			o.scale(kx, ky);
			o.setTopClass(0, 1.0);
			o.setZ(0.0);
			o.setText(m_className);
			o.setTstamp(m_pT->getTfrom());

			m_pU->add(o);
		}

		m_pU->swap();
	}

}

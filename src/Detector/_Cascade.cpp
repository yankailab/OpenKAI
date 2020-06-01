/*
 *  Created on: Sept 28, 2016
 *      Author: yankai
 */
#include "_Cascade.h"

#ifdef USE_OPENCV
#ifdef USE_CUDA

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

bool _Cascade::init(void *pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	pK->v("scaleFactor", &m_scaleFactor);
	pK->v("minNeighbors", &m_minNeighbors);
	pK->v("bGPU", &m_bGPU);

	if (m_bGPU)
	{
		m_pGCC = cuda::CascadeClassifier::create(m_modelFile);
		NULL_F(m_pGCC);
	}
	else
	{
		F_ERROR_F(m_CC.load(m_modelFile));
	}

	return true;
}

int _Cascade::check(void)
{
	NULL__(m_pV, -1);
	NULL__(m_pU, -1);

	return 0;
}

bool _Cascade::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG_E(retCode);
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _Cascade::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		if (check() >= 0)
		{
			if (m_bGPU)
				detectGPU();
			else
				detectCPU();
		}

		this->autoFPSto();
	}

}

void _Cascade::detectGPU(void)
{
	Frame *pGray = m_pV->BGR();
	NULL_(pGray);
	IF_(pGray->bEmpty());
	GpuMat m;
	pGray->gm()->copyTo(m);

	int minSize = m.cols * m_pU->m_rArea.x;
	int maxSize = m.cols * m_pU->m_rArea.y;

	m_pGCC->setFindLargestObject(false);
	m_pGCC->setScaleFactor(m_scaleFactor);
	m_pGCC->setMinNeighbors(m_minNeighbors);
	m_pGCC->setMinObjectSize(cv::Size(minSize, minSize));
	m_pGCC->setMaxObjectSize(cv::Size(maxSize, maxSize));

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
		o.normalize(kx, ky);
		o.setTopClass(0, 1.0);
		o.setZ(0.0);
		o.setText(m_className);
		o.m_tStamp = m_tStamp;

		m_pU->add(o);
	}

	m_pU->updateObj();
}

void _Cascade::detectCPU(void)
{
	Frame *pGray = m_pV->BGR();
	NULL_(pGray);
	IF_(pGray->bEmpty());
	Mat m;
	pGray->m()->copyTo(m);

	int minSize = m.cols * m_pU->m_rArea.x;
	int maxSize = m.cols * m_pU->m_rArea.y;

	vector<Rect> vRect;
	_Object o;
	float kx = 1.0 / m.cols;
	float ky = 1.0 / m.rows;

	m_CC.detectMultiScale(m, vRect, m_scaleFactor, m_minNeighbors,
			0 | CASCADE_SCALE_IMAGE, Size(minSize, minSize),
			Size(maxSize, maxSize));

	for (int i = 0; i < vRect.size(); i++)
	{
		o.setBB2D(rect2BB<vFloat4>(vRect[i]));
		o.normalize(kx, ky);
		o.setTopClass(0, 1.0);
		o.setZ(0.0);
		o.setText(m_className);
		o.m_tStamp = m_tStamp;

		m_pU->add(o);
	}

	m_pU->updateObj();
}

void _Cascade::draw(void)
{
	this->_DetectorBase::draw();

}

}
#endif
#endif


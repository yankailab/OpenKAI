/*
 *  Created on: Sept 28, 2016
 *      Author: yankai
 */
#include "_Cascade.h"

#ifdef USE_OPENCV
#ifdef USE_CASCADE
#ifdef USE_CUDA

namespace kai
{
_Cascade::_Cascade()
{
	m_minSize = 0.0;
	m_maxSize = 1.0;
	m_scaleFactor = 1.1;
	m_minNeighbors = 3;
	m_area.init();
	m_area.z = 1.0;
	m_area.w = 1.0;
	m_className = "";
	m_bGPU = true;
}

_Cascade::~_Cascade()
{
}

bool _Cascade::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("scaleFactor", &m_scaleFactor);
	pK->v("minNeighbors", &m_minNeighbors);
	pK->v("bGPU", &m_bGPU);

	F_INFO(pK->v("left", &m_area.x));
	F_INFO(pK->v("top", &m_area.y));
	F_INFO(pK->v("right", &m_area.z));
	F_INFO(pK->v("bottom", &m_area.w));

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

		this->_DetectorBase::update();

		if(m_bGPU)
		{
			detectGPU();
		}
		else
		{
			detectCPU();
		}

		this->autoFPSto();
	}

}

void _Cascade::detectGPU(void)
{
	NULL_(m_pVision);

	Frame* pGray = m_pVision->gray();
	NULL_(pGray);
	IF_(pGray->empty());
	GpuMat m;
	pGray->getGMat()->copyTo(m);

	int minSize = m.cols * m_minSize;
	int maxSize = m.cols * m_maxSize;

	m_pGCC->setFindLargestObject(false);
	m_pGCC->setScaleFactor(m_scaleFactor);
	m_pGCC->setMinNeighbors(m_minNeighbors);
	m_pGCC->setMinObjectSize(cv::Size(minSize,minSize));
	m_pGCC->setMaxObjectSize(cv::Size(maxSize,maxSize));

	vector<Rect> vRect;
	cv::cuda::GpuMat gFound;
	m_pGCC->detectMultiScale(m, gFound);
	m_pGCC->convert(gFound, vRect);

	OBJECT obj;
	uint64_t tNow = get_time_usec();

	for (int i = 0; i < vRect.size(); i++)
	{
		rect2vInt4(&vRect[i], &obj.m_bbox);
		obj.m_camSize.x = m.cols;
		obj.m_camSize.y = m.rows;
		obj.i2fBBox();
		obj.m_iClass = 0;
		obj.m_dist = 0.0;
		obj.m_name = m_className;
		obj.m_frameID = tNow;

		add(&obj);
	}
}

void _Cascade::detectCPU(void)
{
	NULL_(m_pVision);

	Frame* pGray = m_pVision->gray();
	NULL_(pGray);
	IF_(pGray->empty());
	Mat m;
	pGray->getCMat()->copyTo(m);

	int minSize = m.cols * m_minSize;
	int maxSize = m.cols * m_maxSize;

	vector<Rect> vRect;
	OBJECT obj;

	m_CC.detectMultiScale(m,
						  vRect,
						  m_scaleFactor,
						  m_minNeighbors,
						  0 | CASCADE_SCALE_IMAGE,
						  Size(minSize, minSize),
						  Size(maxSize, maxSize));

	uint64_t tNow = get_time_usec();

	for (int i = 0; i < vRect.size(); i++)
	{
		rect2vInt4(&vRect[i], &obj.m_bbox);
		obj.m_camSize.x = m.cols;
		obj.m_camSize.y = m.rows;
		obj.i2fBBox();
		obj.m_iClass = 0;
		obj.m_dist = 0.0;
		obj.m_name = m_className;
		obj.m_frameID = tNow;

		add(&obj);
	}
}

void _Cascade::draw(void)
{
	this->_DetectorBase::draw();

}

}
#endif
#endif
#endif


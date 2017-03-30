/*
 *  Created on: Sept 28, 2016
 *      Author: yankai
 */
#include "_Cascade.h"

namespace kai
{
_Cascade::_Cascade()
{
	m_minSize = 0.0;
	m_maxSize = 1.0;
	m_overlapMin = 1.0;
	m_area.init();
	m_area.z = 1.0;
	m_area.w = 1.0;
	m_className = "";
}

_Cascade::~_Cascade()
{
}

bool _Cascade::init(void* pKiss)
{
	IF_F(!this->_AIbase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("className", &m_className));
	F_INFO(pK->v("minSize", &m_minSize));
	F_INFO(pK->v("maxSize", &m_maxSize));
	F_INFO(pK->v("overlapMin", &m_overlapMin));

	F_INFO(pK->v("left", &m_area.x));
	F_INFO(pK->v("top", &m_area.y));
	F_INFO(pK->v("right", &m_area.z));
	F_INFO(pK->v("bottom", &m_area.w));

	F_ERROR_F(m_CC.load(m_fileModel));

//	if (m_device == CASCADE_CPU)
//	{
//		F_ERROR_F(m_CC.load(presetDir+m_fileModel));
//	}
//	else if (m_device == CASCADE_CUDA)
//	{
//		m_pCascade = cuda::CascadeClassifier::create(cascadeFile);
//		F_ERROR_F(m_pCascade);
//		//TODO:set the upper limit of objects to be detected
//		F_INFO(pConfig->v("cudaDeviceID", &m_cudaDeviceID));
//	}

	return true;
}

bool _Cascade::link(void)
{
	IF_F(!this->_AIbase::link());
	Kiss* pK = (Kiss*) m_pKiss;

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

		detect();

		this->autoFPSto();
	}

}

void _Cascade::detect(void)
{
	NULL_(m_pStream);

	Frame* pGray = m_pStream->gray();
	NULL_(pGray);
	IF_(pGray->empty());
	Mat m;
	pGray->getCMat()->copyTo(m);

	int minSize = m.cols * m_minSize;
	int maxSize = m.cols * m_maxSize;

	vector<Rect> vRect;
	OBJECT obj;

	m_CC.detectMultiScale(m, vRect, 1.1, 3, 0 | CASCADE_SCALE_IMAGE, Size(minSize, minSize), Size(maxSize, maxSize));
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

		addOrUpdate(&obj);
	}
}

void _Cascade::addOrUpdate(OBJECT* pNewObj)
{
	NULL_(pNewObj);

	for (int i = 0; i < m_nObj; i++)
	{
		OBJECT* pObj = get(i, 0);
		IF_CONT(!pObj);
		IF_CONT(pObj->m_frameID <= 0);
		IF_CONT(overlapRatio(&pObj->m_bbox, &pNewObj->m_bbox) < m_overlapMin);

		*pObj = *pNewObj;
		return;
	}

	add(pNewObj);
}

bool _Cascade::draw(void)
{
	IF_F(!this->_AIbase::draw());

	return true;
}

}

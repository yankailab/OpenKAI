/*
 *  Created on: Mar 7, 2018
 *      Author: yankai
 */
#include "_YOLO.h"

#ifdef USE_DARKNET

namespace kai
{

_YOLO::_YOLO()
{
	m_pRGBA = NULL;
	m_pRGBAf = NULL;
	m_minSize = 0.0;
	m_maxSize = 1.0;
	m_area.init();
	m_area.z = 1.0;
	m_area.w = 1.0;

	m_nBox = 0;
	m_nBoxMax = 0;
	m_nClass = 0;

	m_className = "";
}

_YOLO::~_YOLO()
{
	DEL(m_pRGBA);
	DEL(m_pRGBAf);
}

bool _YOLO::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK, className);
	KISSm(pK, minSize);
	KISSm(pK, maxSize);

	F_INFO(pK->v("l", &m_area.x));
	F_INFO(pK->v("t", &m_area.y));
	F_INFO(pK->v("r", &m_area.z));
	F_INFO(pK->v("b", &m_area.w));

	m_pRGBA = new Frame();
	m_pRGBAf = new Frame();

	bSetActive(true);

	return true;
}

bool _YOLO::link(void)
{
	IF_F(!this->_DetectorBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	return true;
}

bool _YOLO::start(void)
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

void _YOLO::update(void)
{
	apiYOLO();
	return;

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_DetectorBase::update();

		m_obj.update();
		detect();

		updateStatistics();

		this->autoFPSto();
	}

}

void _YOLO::detect(void)
{
	NULL_(m_pVision);

	Frame* pBGR = m_pVision->bgr();
	NULL_(pBGR);
	IF_(pBGR->empty());
	IF_(m_pRGBA->isNewerThan(pBGR));

	m_pRGBA->getRGBAOf(pBGR);
	GpuMat* pGMat = m_pRGBA->getGMat();
	IF_(pGMat->empty());

	GpuMat fGMat;
	pGMat->convertTo(fGMat, CV_32FC4);

	m_nBox = m_nBoxMax;

	int camArea = fGMat.cols * fGMat.rows;
	int minSize = camArea * m_minSize;
	int maxSize = camArea * m_maxSize;

	m_tStamp = getTimeUsec();

	OBJECT obj;
	for (int n = 0; n < m_nBox; n++)
	{
		obj.init();
		obj.addClass(0);
		obj.m_tStamp = m_tStamp;

		obj.m_bbox.x = 0;
		obj.m_bbox.y = 0;
		obj.m_bbox.z = 0;
		obj.m_bbox.w = 0;
		obj.m_camSize.x = fGMat.cols;
		obj.m_camSize.y = fGMat.rows;
		if(obj.m_bbox.x < 0)obj.m_bbox.x = 0;
		if(obj.m_bbox.y < 0)obj.m_bbox.y = 0;
		if(obj.m_bbox.z > obj.m_camSize.x)obj.m_bbox.z = obj.m_camSize.x;
		if(obj.m_bbox.w > obj.m_camSize.y)obj.m_bbox.w = obj.m_camSize.y;
		obj.i2fBBox();

		int oSize = obj.m_bbox.area();
		IF_CONT(oSize < minSize);
		IF_CONT(oSize > maxSize);

		add(&obj);

		LOG_I("BBox: "<< i2str(obj.m_iClass) << " Prob: " << f2str(0.0));
	}

}

bool _YOLO::draw(void)
{
	IF_F(!this->_DetectorBase::draw());

	return true;
}

}

#endif


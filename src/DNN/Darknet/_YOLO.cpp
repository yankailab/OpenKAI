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
	m_thresh = 0.24;
	m_hier = 0.5;
	m_nms = 0.4;
	m_nPredAvr = 3;

	m_pYoloObj = NULL;
	m_nBatch = 1;
	m_pBGR = NULL;
}

_YOLO::~_YOLO()
{
	DEL(m_pBGR);
	DEL(m_pYoloObj);
}

bool _YOLO::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSdm(pK,thresh);
	KISSdm(pK,hier);
	KISSdm(pK,nms);
	KISSm(pK,nBatch);
	KISSm(pK,nPredAvr);

	IF_Fl(!yoloInit( m_modelFile.c_str(),
					m_trainedFile.c_str(),
					m_labelFile.c_str(),
					m_nPredAvr,
					m_nBatch), "YOLO init failed");

	m_pYoloObj = new yolo_object[DETECTOR_N_OBJ];
	m_pBGR = new Frame();

	m_nClass = yoloNClass();
	for(int i=0; i<m_nClass; i++)
	{
		m_pClassStatis[i].init();
		m_pClassStatis[i].m_name = yoloGetClassName(i);
	}

	bSetActive(true);
	m_bReady = true;
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
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_DetectorBase::update();

		m_obj.update();
		detect();

		this->autoFPSto();
	}

}

void _YOLO::detect(void)
{
	NULL_(m_pVision);

	Frame* pBGR = m_pVision->bgr();
	NULL_(pBGR);
	IF_(pBGR->empty());
	IF_(!pBGR->isNewerThan(m_pBGR));
	m_pBGR->update(pBGR);

	Mat* pMat = m_pBGR->getCMat();
	IplImage ipl = *pMat;
	int nDetected = yoloUpdate(&ipl, m_pYoloObj, DETECTOR_N_OBJ, (float)m_thresh, (float)m_hier, (float)m_nms);
	IF_(nDetected <= 0);

	m_tStamp = getTimeUsec();

	OBJECT obj;
	for (int i = 0; i < nDetected; i++)
	{
		yolo_object* pYO = &m_pYoloObj[i];

		obj.init();
		obj.m_tStamp = m_tStamp;
		obj.setClassMask(pYO->m_mClass);
		obj.setTopClass(pYO->m_iClass);

		obj.m_fBBox.x = (double)pYO->m_l;
		obj.m_fBBox.y = (double)pYO->m_t;
		obj.m_fBBox.z = (double)pYO->m_r;
		obj.m_fBBox.w = (double)pYO->m_b;
		obj.m_camSize.x = pMat->cols;
		obj.m_camSize.y = pMat->rows;
		obj.f2iBBox();
		if(obj.m_bbox.x < 0)obj.m_bbox.x = 0;
		if(obj.m_bbox.y < 0)obj.m_bbox.y = 0;
		if(obj.m_bbox.z > obj.m_camSize.x)obj.m_bbox.z = obj.m_camSize.x;
		if(obj.m_bbox.w > obj.m_camSize.y)obj.m_bbox.w = obj.m_camSize.y;

		add(&obj);

		LOG_I("Class: "<< i2str(obj.m_iClass));
	}
}

bool _YOLO::draw(void)
{
	IF_F(!this->_DetectorBase::draw());

	return true;
}

}
#endif

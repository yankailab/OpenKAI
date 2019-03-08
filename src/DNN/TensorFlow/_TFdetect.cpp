/*
 *  Created on: Aug 29, 2018
 *      Author: yankai
 */
#include "_TFdetect.h"

#ifdef USE_TENSORFLOW

namespace kai
{

_TFdetect::_TFdetect()
{
	m_nBatch = 1;
}

_TFdetect::~_TFdetect()
{
}

bool _TFdetect::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, nPredAvr);

	IF_Fl(!yoloInit(m_modelFile.c_str(), m_trainedFile.c_str(),
					m_labelFile.c_str(), m_nPredAvr, m_nBatch),
			"YOLO init failed");

	m_pYoloObj = new yolo_object[OBJECT_N_OBJ];

	m_nClass = yoloNClass();
	for (int i = 0; i < m_nClass; i++)
	{
		m_pClassStatis[i].init();
		m_pClassStatis[i].m_name = yoloGetClassName(i);
	}

	bSetActive(true);
	m_bReady = true;
	return true;
}

bool _TFdetect::start(void)
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

void _TFdetect::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();
		m_obj.update();
		this->_DetectorBase::update();

		this->autoFPSto();
	}

}

void _TFdetect::detect(void)
{
	NULL_(m_pVision);

	Frame* pBGR = m_pVision->BGR();
	NULL_(pBGR);
	IF_(pBGR->bEmpty());
	IF_(pBGR->tStamp() <= m_BGR.tStamp());
	m_BGR = *pBGR;
	Mat mBGR = *m_BGR.m();

	vInt4 iRoi;
	iRoi.x = mBGR.cols * m_roi.x;
	iRoi.y = mBGR.rows * m_roi.y;
	iRoi.z = mBGR.cols * m_roi.z;
	iRoi.w = mBGR.rows * m_roi.w;
	Rect rRoi;
	vInt42rect(iRoi, rRoi);

	vInt2 cSize;
	cSize.x = mBGR.cols;
	cSize.y = mBGR.rows;

	IplImage ipl = mBGR(rRoi);
	int nDet = yoloUpdate(&ipl,
						  m_pYoloObj,
						  OBJECT_N_OBJ,
						  (float) m_thresh,
						  (float) m_hier,
						  (float) m_nms);
	IF_(nDet <= 0);

	OBJECT obj;
	for (int i = 0; i < nDet; i++)
	{
		yolo_object* pYO = &m_pYoloObj[i];

		obj.init();
		obj.m_tStamp = m_tStamp;
		obj.setClassMask(pYO->m_mClass);
		obj.setTopClass(pYO->m_topClass, (double) pYO->m_topProb);

		vInt4 iBB;
		iBB.x = rRoi.x + rRoi.width * pYO->m_l;
		iBB.y = rRoi.y + rRoi.height * pYO->m_t;
		iBB.z = rRoi.x + rRoi.width * pYO->m_r;
		iBB.w = rRoi.y + rRoi.height * pYO->m_b;
		obj.setBB(iBB,cSize);

		this->add(&obj);
		LOG_I("Class: " + i2str(obj.m_topClass));
	}

	m_tStamp = getTimeUsec();
}

bool _TFdetect::draw(void)
{
	IF_F(!this->_DetectorBase::draw());

	return true;
}

}
#endif

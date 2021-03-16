/*
 *  Created on: Mar 7, 2018
 *      Author: yankai
 */
#include "_YOLO.h"

#ifdef USE_OPENCV
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
}

_YOLO::~_YOLO()
{
	DEL(m_pYoloObj);
}

bool _YOLO::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, thresh);
	KISSm(pK, hier);
	KISSm(pK, nms);
	KISSm(pK, nBatch);
	KISSm(pK, nPredAvr);

	IF_Fl(!yoloInit(m_fModel.c_str(), m_fWeight.c_str(),
					m_labelFile.c_str(), m_nPredAvr, m_nBatch),
			"YOLO init failed");

	m_pYoloObj = new yolo_object[OBJECT_N_OBJ];

	m_nClass = yoloNClass();
	for (int i = 0; i < m_nClass; i++)
	{
		OBJECT_CLASS oc;
		oc.init();
		oc.m_name = yoloGetClassName(i);
		m_vClass.push_back(oc);
	}

	return true;
}

bool _YOLO::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

void _YOLO::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		IF_CONT(!detect());

		m_obj.update();

		if(m_bGoSleep)
		{
			m_obj.m_pPrev->reset();
		}

		m_pT->autoFPSto();
	}
}

bool _YOLO::detect(void)
{
	NULL_F(m_pVision);
	Frame* pBGR = m_pVision->BGR();
	NULL_F(pBGR);
	IF_F(pBGR->bEmpty());
	IF_F(pBGR->tStamp() <= m_BGR.tStamp());
	m_BGR.copy(*pBGR);
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

	OBJECT obj;
	for (int i = 0; i < nDet; i++)
	{
		yolo_object* pYO = &m_pYoloObj[i];

		obj.init();
		obj.m_tStamp = m_pT->getTfrom();
		obj.setClassMask(pYO->m_mClass);
		obj.setTopClass(pYO->m_topClass, (double) pYO->m_topProb);

		vInt4 iBB;
		iBB.x = rRoi.x + rRoi.width * pYO->m_l;
		iBB.y = rRoi.y + rRoi.height * pYO->m_t;
		iBB.z = rRoi.x + rRoi.width * pYO->m_r;
		iBB.w = rRoi.y + rRoi.height * pYO->m_b;
		obj.setBB(iBB,cSize);

		this->add(&obj);
		LOG_I("Class: " + i2str(obj.getTopClass()));
	}

	return true;
}

bool _YOLO::draw(void)
{
	IF_F(!this->_DetectorBase::draw());

	return true;
}

}
#endif
#endif

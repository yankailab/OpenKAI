/*
 *  Created on: Sept 28, 2016
 *      Author: yankai
 */
#include "_AIbase.h"


namespace kai
{
_AIbase::_AIbase()
{
	_ThreadBase();

	m_pStream = NULL;
	m_pObj = NULL;

	m_fileModel = "";
	m_fileTrained = "";
	m_fileMean = "";
	m_fileLabel = "";
}

_AIbase::~_AIbase()
{
	DEL(m_pObj);
}

bool _AIbase::init(void* pKiss)
{
	CHECK_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	//Setup model
	string modelDir = "";
	string presetDir = "";

	F_INFO(pK->root()->o("APP")->v("presetDir", &presetDir));
	F_INFO(pK->v("dir", &modelDir));

	F_INFO(pK->v("modelFile", &m_fileModel));
	F_INFO(pK->v("trainedFile", &m_fileTrained));
	F_INFO(pK->v("meanFile", &m_fileMean));
	F_INFO(pK->v("labelFile", &m_fileLabel));

	m_fileModel = modelDir + m_fileModel;
	m_fileTrained = modelDir + m_fileTrained;
	m_fileMean = modelDir + m_fileMean;
	m_fileLabel = modelDir + m_fileLabel;

	m_pObj = new Object();

	return true;
}

bool _AIbase::link(void)
{
	CHECK_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_ERROR_F(pK->v("_Stream", &iName));
	m_pStream = (_StreamBase*) (pK->root()->getChildInstByName(&iName));

	return true;
}

bool _AIbase::start(void)
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

void _AIbase::update(void)
{
	NULL_(m_pStream);
}

bool _AIbase::draw(void)
{
	CHECK_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	OBJECT* pObj;
	int i=0;
	while((pObj=m_pObj->get(i++)))
	{
		Rect bbox;
		bbox.x = pObj->m_bbox.m_x;
		bbox.y = pObj->m_bbox.m_y;
		bbox.width = pObj->m_bbox.m_z - pObj->m_bbox.m_x;
		bbox.height = pObj->m_bbox.m_w - pObj->m_bbox.m_y;

		rectangle(*pMat, bbox, Scalar(0, 255, 0), 1);

		putText(*pMat, pObj->m_name,
				Point(bbox.x + bbox.width / 2, bbox.y + bbox.height / 2),
				FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0,255,0), 1);
	}

	return true;
}

}


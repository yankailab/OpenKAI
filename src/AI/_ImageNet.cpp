/*
 *  Created on: Sept 28, 2016
 *      Author: yankai
 */
#include "_ImageNet.h"

#ifdef USE_TENSORRT

namespace kai
{
_ImageNet::_ImageNet()
{
	_AIbase();

	m_pIN = NULL;
	m_nBatch = 1;
	m_blobIn = "data";
	m_blobOut = "prob";
}

_ImageNet::~_ImageNet()
{
	this->~_AIbase();
}

bool _ImageNet::init(void* pKiss)
{
	CHECK_F(!this->_AIbase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("nBatch", &m_nBatch));
	F_INFO(pK->v("blobIn", &m_blobIn));
	F_INFO(pK->v("blobOut", &m_blobOut));

	m_pRGBA = new Frame();

	return true;
}

bool _ImageNet::link(void)
{
	CHECK_F(!this->_AIbase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	return true;
}

bool _ImageNet::start(void)
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

void _ImageNet::update(void)
{
	m_pIN = imageNet::Create(m_fileModel.c_str(), m_fileTrained.c_str(),
			m_fileMean.c_str(), m_fileLabel.c_str(), m_blobIn.c_str(),
			m_blobOut.c_str());
	NULL_(m_pIN);

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();

		this->autoFPSto();
	}

}

void _ImageNet::detect(void)
{
	NULL_(m_pStream);
	NULL_(m_pIN);

	Frame* pBGR = m_pStream->bgr();
	NULL_(pBGR);
	CHECK_(pBGR->empty());
	CHECK_(m_pRGBA->isNewerThan(pBGR));

	m_pRGBA->getRGBAOf(pBGR);
	GpuMat* pGMat = m_pRGBA->getGMat();
	CHECK_(pGMat->empty());

	GpuMat fGMat;
	pGMat->convertTo(fGMat, CV_32FC4);

	float confidence = 0.0f;
	int img_class = m_pIN->Classify((float*) fGMat.data, fGMat.cols, fGMat.rows,
			&confidence);

	if (img_class >= 0)
	{
		printf(">>>ImageNet:  %2.5f%% class #%i (%s)\n",
				confidence * 100.0f, img_class, m_pIN->GetClassDesc(img_class));

	}

//	LOG_I("Detected BBox: "<<m_nBox);
/*
	m_pObj->reset();

	OBJECT obj;
	for (int n = 0; n < m_nBox; n++)
	{
		const int nc = m_confCPU[n * 2 + 1];
		float* bb = m_bbCPU + (n * 4);

		obj.m_iClass = 0;
		obj.m_bbox.m_x = (int) bb[0];
		obj.m_bbox.m_y = (int) bb[1];
		obj.m_bbox.m_z = (int) bb[2];
		obj.m_bbox.m_w = (int) bb[3];
		obj.m_camSize.m_x = fGMat.cols;
		obj.m_camSize.m_y = fGMat.rows;
		obj.m_dist = 0.0;
		obj.m_prob = 0.0;
		obj.m_name = m_className;

		m_pObj->add(&obj);
	}
*/
}

bool _ImageNet::draw(void)
{
	CHECK_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	OBJECT* pObj;
	int i = 0;
	while ((pObj = m_pObj->get(i++)))
	{
		Rect bbox;
		bbox.x = pObj->m_bbox.m_x;
		bbox.y = pObj->m_bbox.m_y;
		bbox.width = pObj->m_bbox.m_z - pObj->m_bbox.m_x;
		bbox.height = pObj->m_bbox.m_w - pObj->m_bbox.m_y;

		rectangle(*pMat, bbox, Scalar(0, 255, 0), 1);

		putText(*pMat, pObj->m_name,
				Point(bbox.x + bbox.width / 2, bbox.y + bbox.height / 2),
				FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 0), 1);
	}

	return true;
}

}

#endif

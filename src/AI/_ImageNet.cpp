/*
 *  Created on: Sept 28, 2016
 *      Author: yankai
 */
#include "_ImageNet.h"

namespace kai
{

_ImageNet::_ImageNet()
{
#ifdef USE_TENSORRT
	m_pIN = NULL;
#endif

	m_mode = noThread;
	m_pRGBA = NULL;
	m_nBatch = 1;
	m_blobIn = "data";
	m_blobOut = "prob";

	m_fDist = 0.0;
	m_detectMinSize = 0.0;
	m_detectMaxSize = 0.0;
	m_extraBBox = 0.0;
}

_ImageNet::~_ImageNet()
{
	DEL(m_pRGBA);
#ifdef USE_TENSORRT
	DEL(m_pIN);
#endif

}

bool _ImageNet::init(void* pKiss)
{
	CHECK_F(!this->_AIbase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	string presetDir = "";
	F_INFO(pK->root()->o("APP")->v("presetDir", &presetDir));
	F_INFO(pK->v("fDist", &m_fDist));
	F_INFO(pK->v("detectMinSize", &m_detectMinSize));
	F_INFO(pK->v("detectMaxSize", &m_detectMaxSize));
	F_INFO(pK->v("extraBBox", &m_extraBBox));

	F_INFO(pK->v("nBatch", &m_nBatch));
	F_INFO(pK->v("blobIn", &m_blobIn));
	F_INFO(pK->v("blobOut", &m_blobOut));

	m_pRGBA = new Frame();

	string iName = "noThread";
	F_INFO(pK->v("mode", &iName));
	if(iName == "depth")
		m_mode = depth;
	else if(iName == "object")
		m_mode = object;

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
#ifdef USE_TENSORRT
	m_pIN = imageNet::Create(m_fileModel.c_str(), m_fileTrained.c_str(),
			m_fileMean.c_str(), m_fileLabel.c_str(), m_blobIn.c_str(),
			m_blobOut.c_str());

	NULL_(m_pIN);
#endif

	CHECK_(m_mode==noThread);

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		if(m_mode == depth)
		{
			detectDepth();
		}
		else if(m_mode == object)
		{
			detectObject();
		}
		else
		{
			return;
		}

		this->autoFPSto();
	}
}

void _ImageNet::detectObject(void)
{
	NULL_(m_pStream);
	NULL_(m_pIN);
	Frame* pBGR = m_pStream->bgr();
	NULL_(pBGR);
	CHECK_(pBGR->empty());
	GpuMat gBGR = *pBGR->getGMat();

	for (int i = 0; i < m_iObj; i++)
	{
		OBJECT* pObj = &m_pObj[i];
		if(pObj->m_bbox.area()<=0)
		{
			pObj->m_bbox.m_x = pObj->m_fBBox.m_x * gBGR.cols;
			pObj->m_bbox.m_y = pObj->m_fBBox.m_y * gBGR.rows;
			pObj->m_bbox.m_z = pObj->m_fBBox.m_z * gBGR.cols;
			pObj->m_bbox.m_w = pObj->m_fBBox.m_w * gBGR.rows;
		}
		if(pObj->m_bbox.area()<=0)continue;

		if(!gBGR.empty())
		{
			Rect bb;
			GpuMat gBB;
			GpuMat gfBB;

			vInt42rect(&pObj->m_bbox,&bb);
			gBB = GpuMat(gBGR,bb);
			gBB.convertTo(gfBB, CV_32FC3);

#ifdef USE_TENSORRT
			float prob = 0;
			pObj->m_iClass = m_pIN->Classify((float*) gfBB.data, gfBB.cols, gfBB.rows, &prob);
			if (pObj->m_iClass >= 0)
			{
				pObj->m_name = m_pIN->GetClassDesc(pObj->m_iClass);

				std::string::size_type k;
				k = pObj->m_name.find(',');
				if (k != std::string::npos)
					pObj->m_name.erase(k);
			}

#endif
		}

		pObj->m_frameID = get_time_usec();
	}
}


void _ImageNet::detectDepth(void)
{
	NULL_(m_pStream);
	Frame* pDepth = m_pStream->depth();
	NULL_(pDepth);
	CHECK_(pDepth->empty());
	GpuMat gD = *(pDepth->getGMat());
	GpuMat gD2;

	Frame* pBGR = m_pStream->bgr();
	GpuMat gBGR = *pBGR->getGMat();

	//MinSize
	double minSize = m_detectMinSize * gD.cols * gD.rows;
	double maxSize = m_detectMaxSize * gD.cols * gD.rows;

#ifndef USE_OPENCV4TEGRA
	cuda::threshold(gD, gD2, m_fDist * 255.0, 255, cv::THRESH_BINARY);
#else
	gpu::threshold(gD, gD2, m_fDist * 255.0, 255, cv::THRESH_BINARY);
#endif

	Mat cMat;
	gD2.download(cMat);

	// find contours
	// findContours will modify the contents of the given Mat
	vector<vector<Point> > contours;
	findContours(cMat, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	// Approximate contours to polygons + get bounding rects
	vector<Point> contourPoly;
	int extraX = cMat.cols*m_extraBBox;
	int extraY = cMat.rows*m_extraBBox;

	for (unsigned int i = 0; i < contours.size(); i++)
	{
		approxPolyDP(Mat(contours[i]), contourPoly, 3, true);
		Rect bb = boundingRect(Mat(contourPoly));
		if (bb.area() < minSize)
			continue;
		if(bb.area() > maxSize)
			continue;

		OBJECT obj;
		obj.m_camSize.m_x = cMat.cols;
		obj.m_camSize.m_y = cMat.rows;
		if(m_bDrawContour)
			obj.m_contour = contourPoly;

		obj.m_bbox.m_x = bb.x - extraX;
		obj.m_bbox.m_y = bb.y - extraY;
		obj.m_bbox.m_z = bb.x + bb.width + extraX;
		obj.m_bbox.m_w = bb.y + bb.height + extraY;
		if(obj.m_bbox.m_x < 0)obj.m_bbox.m_x = 0;
		if(obj.m_bbox.m_y < 0)obj.m_bbox.m_y = 0;
		if(obj.m_bbox.m_z > cMat.cols)obj.m_bbox.m_z = cMat.cols;
		if(obj.m_bbox.m_w > cMat.rows)obj.m_bbox.m_w = cMat.rows;

		//classify
		obj.m_iClass = -1;
		obj.m_name = "?";
		if(!gBGR.empty())
		{
			GpuMat gBB;
			GpuMat gfBB;

			gBB = GpuMat(gBGR,bb);
			gBB.convertTo(gfBB, CV_32FC3);

#ifdef USE_TENSORRT
			float prob = 0;
			obj.m_iClass = m_pIN->Classify((float*) gfBB.data, gfBB.cols, gfBB.rows, &prob);
			if (obj.m_iClass >= 0)
			{
				obj.m_name = m_pIN->GetClassDesc(obj.m_iClass);

				std::string::size_type k;
				k = obj.m_name.find(',');
				if (k != std::string::npos)
					obj.m_name.erase(k);
			}

#endif

			LOG_I(obj.m_name);
		}

		obj.m_frameID = get_time_usec();
		add(&obj);
	}
}

int _ImageNet::classify(Frame* pImg, string* pName)
{
	if(!pImg)return -1;
	GpuMat* gM = pImg->getGMat();
	if(!gM)return -1;
	if(gM->empty())return -1;

	GpuMat gfM;
	gM->convertTo(gfM, CV_32FC3);

	int iClass = -1;
#ifdef USE_TENSORRT
	if(!m_pIN)return -1;
	float prob = 0;
	iClass = m_pIN->Classify((float*) gfM.data, gfM.cols, gfM.rows, &prob);

	if (iClass < 0)
		return -1;

	if(pName)
	{
		*pName = m_pIN->GetClassDesc(iClass);

		std::string::size_type k;
		k = pName->find(',');
		if (k != std::string::npos)
			pName->erase(k);
	}
#endif

	return iClass;
}

bool _ImageNet::draw(void)
{
	CHECK_F(!this->_AIbase::draw());

	return true;
}


}

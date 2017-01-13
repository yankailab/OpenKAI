/*
 *  Created on: Sept 28, 2016
 *      Author: yankai
 */
#include "_ImageNet.h"

namespace kai
{
_ImageNet::_ImageNet()
{
	_AIbase();

#ifdef USE_TENSORRT
	m_pIN = NULL;
#endif

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
	this->~_AIbase();
	DEL(m_pObj);
	DEL(m_pRGBA);
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
	Frame* pDepth = ((_ZED*)m_pStream)->norm();//m_pStream->depth();
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

//#ifndef USE_OPENCV4TEGRA
//	cuda::multiply(gD, Scalar(m_fDist), gD2);
//#else
//	gpu::multiply(gD, Scalar(m_fDist), gD2);
//#endif
//	gD2.convertTo(gD,CV_8U);

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

bool _ImageNet::draw(void)
{
	CHECK_F(!this->_AIbase::draw());

	return true;
}


}

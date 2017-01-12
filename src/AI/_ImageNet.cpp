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

	m_pRGBA = NULL;
	m_nBatch = 1;
	m_blobIn = "data";
	m_blobOut = "prob";

	m_pStream = NULL;
	m_pObj = NULL;
	m_nObj = 128;
	m_iObj = 0;
	m_obsLifetime = USEC_1SEC;

	m_fDist = 0.0;
	m_detectMinSize = 0.0;
	m_extraBBox = 0.0;
	m_bDrawContour = false;
	m_contourBlend = 0.125;

	m_sizeName = 0.5;
	m_sizeDist = 0.5;
	m_colName = Scalar(255,255,0);
	m_colDist = Scalar(0,255,255);
	m_colObs = Scalar(255,255,0);
}

_ImageNet::~_ImageNet()
{
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
	F_INFO(pK->v("extraBBox", &m_extraBBox));
	F_INFO(pK->v("nObs", &m_nObj));
	F_INFO(pK->v("obsLifetime", (int*)&m_obsLifetime));
	F_INFO(pK->v("bDrawContour", &m_bDrawContour));
	F_INFO(pK->v("contourBlend", &m_contourBlend));

	F_INFO(pK->v("sizeName", &m_sizeName));
	F_INFO(pK->v("sizeDist", &m_sizeDist));

	F_INFO(pK->v("nameB", &m_colName[0]));
	F_INFO(pK->v("nameG", &m_colName[1]));
	F_INFO(pK->v("nameR", &m_colName[2]));

	F_INFO(pK->v("distB", &m_colDist[0]));
	F_INFO(pK->v("distG", &m_colDist[1]));
	F_INFO(pK->v("distR", &m_colDist[2]));

	F_INFO(pK->v("obsB", &m_colObs[0]));
	F_INFO(pK->v("obsG", &m_colObs[1]));
	F_INFO(pK->v("obsR", &m_colObs[2]));

	F_INFO(pK->v("nBatch", &m_nBatch));
	F_INFO(pK->v("blobIn", &m_blobIn));
	F_INFO(pK->v("blobOut", &m_blobOut));

	m_pRGBA = new Frame();

	m_pObj = new OBJECT[m_nObj];
	for (int i = 0; i < m_nObj; i++)
	{
		m_pObj[i].m_frameID = 0;
		m_pObj[i].m_dist = -1.0;
	}
	m_iObj = 0;

	return true;
}

bool _ImageNet::link(void)
{
	CHECK_F(!this->_AIbase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("_Stream", &iName));
	m_pStream = (_StreamBase*) (pK->root()->getChildInstByName(&iName));

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
	Frame* pDepth = m_pStream->depth();
	NULL_(pDepth);
	CHECK_(pDepth->empty());
	GpuMat gD = *(pDepth->getGMat());
	GpuMat gD2;

	Frame* pBGR = m_pStream->bgr();
	GpuMat gBGR = *pBGR->getGMat();

	//MinSize
	double minSize = m_detectMinSize * gD.cols * gD.rows;

#ifndef USE_OPENCV4TEGRA
	cuda::multiply(gD, Scalar(m_fDist), gD2);
#else
	gpu::multiply(gD, Scalar(m_fDist), gD2);
#endif
	gD2.convertTo(gD,CV_8U);

	Mat cMat;
	gD.download(cMat);

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
/*		if(!gBGR.empty())
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
*/
		obj.m_frameID = get_time_usec();
		add(&obj);
	}

}

bool _ImageNet::add(OBJECT* pNewObj)
{
	NULL_F(pNewObj);
	m_pObj[m_iObj] = *pNewObj;
	if (++m_iObj >= m_nObj)
		m_iObj = 0;
	return true;
}

int _ImageNet::size(void)
{
	return m_nObj;
}

OBJECT* _ImageNet::get(int i, int64_t frameID)
{
	if(frameID - m_pObj[i].m_frameID >= m_obsLifetime)
	{
		return NULL;
	}
	return &m_pObj[i];
}

OBJECT* _ImageNet::getByClass(int iClass)
{
	int i;
	OBJECT* pObj;

	for (i = 0; i < m_nObj; i++)
	{
		pObj = &m_pObj[i];

		if (pObj->m_iClass == iClass)
			return pObj;
	}

	return NULL;
}

bool _ImageNet::draw(void)
{
	CHECK_F(!this->_AIbase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();
	Mat* pMat = pFrame->getCMat();
	CHECK_F(pMat->empty());

	Mat bg;
	if (m_bDrawContour)
	{
		bg = Mat::zeros(Size(pMat->cols, pMat->rows), CV_8UC3);
	}

	uint64_t frameID = get_time_usec() - m_obsLifetime;
	for (int i = 0; i < m_nObj; i++)
	{
		OBJECT* pObj = get(i, frameID);
		if (!pObj)
			continue;
		if(pObj->m_frameID<=0)
			continue;

		Rect r;
		vInt42rect(&pObj->m_bbox, &r);

		if (pObj->m_iClass>=0)
		{
			putText(*pMat, pObj->m_name,
					Point(r.x + r.width / 2, r.y + r.height / 2),
					FONT_HERSHEY_SIMPLEX, m_sizeName, m_colName, 1);
		}

		Scalar colObs = m_colObs;
		int bolObs = 1;

		if (m_bDrawContour)
		{
			drawContours(bg, vector<vector<Point> >(1, pObj->m_contour), -1,
					colObs, CV_FILLED, 8);
		}
		else
		{
			rectangle(*pMat, r, colObs, bolObs);
		}
	}

	if (m_bDrawContour)
	{
		cv::addWeighted(*pMat, 1.0, bg, m_contourBlend, 0.0, *pMat);
	}

	return true;
}


}

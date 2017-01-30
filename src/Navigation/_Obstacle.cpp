/*
 * _Obstacle.cpp
 *
 *  Created on: Jan 6, 2017
 *      Author: yankai
 */

#include "_Obstacle.h"

namespace kai
{

_Obstacle::_Obstacle()
{
	m_pStream = NULL;
	m_pMatrix = NULL;
	m_medianLen = 3;
	m_nFilter = 0;
	m_dBlend = 0.5;

	m_mDim.m_x = 10;
	m_mDim.m_y = 10;
}

_Obstacle::~_Obstacle()
{
	DEL(m_pMatrix);

	for (int i = 0; i < m_nFilter; i++)
		DEL(m_pFilteredMatrix[i]);
}

bool _Obstacle::init(void* pKiss)
{
	CHECK_F(!_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	string presetDir = "";
	F_INFO(pK->root()->o("APP")->v("presetDir", &presetDir));
	F_INFO(pK->v("dBlend", &m_dBlend));

	F_INFO(pK->v("medianLen", &m_medianLen));
	F_INFO(pK->v("matrixW", &m_mDim.m_x));
	F_INFO(pK->v("matrixH", &m_mDim.m_y));

	m_nFilter = m_mDim.area();
	CHECK_F(m_nFilter >= N_FILTER);

	for (int i = 0; i < m_nFilter; i++)
	{
		m_pFilteredMatrix[i] = new Filter();
		m_pFilteredMatrix[i]->startMedian(m_medianLen);
	}

	m_pMatrix = new Frame();

	return true;
}

bool _Obstacle::link(void)
{
	CHECK_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("_Stream", &iName));
	m_pStream = (_StreamBase*) (pK->root()->getChildInstByName(&iName));

	return true;
}

bool _Obstacle::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _Obstacle::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();

		this->autoFPSto();
	}
}

void _Obstacle::detect(void)
{
	NULL_(m_pStream);
	Frame* pDepth = m_pStream->depth();
	NULL_(pDepth);
	CHECK_(pDepth->empty());

	m_pMatrix->getResizedOf(m_pStream->depth(), m_mDim.m_x, m_mDim.m_y);
	Mat* pM = m_pMatrix->getCMat();

	int i,j;
	for(i=0;i<m_mDim.m_y;i++)
	{
		for(j=0;j<m_mDim.m_x;j++)
		{
			m_pFilteredMatrix[i*m_mDim.m_x+j]->input((double)pM->at<uchar>(i,j));
		}
	}
}

double _Obstacle::dist(vDouble4* pROI, vInt2* pPos)
{
	if(!m_pStream)return -1.0;
	if(!pROI)return -1.0;

	vInt4 roi;
	roi.m_x = pROI->m_x * m_mDim.m_x;
	roi.m_y = pROI->m_y * m_mDim.m_y;
	roi.m_z = pROI->m_z * m_mDim.m_x;
	roi.m_w = pROI->m_w * m_mDim.m_y;
	if(roi.m_x<0)roi.m_x=0;
	if(roi.m_y<0)roi.m_y=0;
	if(roi.m_z>=m_mDim.m_x)roi.m_z=m_mDim.m_x-1;
	if(roi.m_w>=m_mDim.m_y)roi.m_w=m_mDim.m_y-1;

	double distMin = 0;
	vInt2 posMin;
	int i,j;
	for(i=roi.m_y;i<roi.m_w;i++)
	{
		for(j=roi.m_x;j<roi.m_z;j++)
		{
			double cellDist = m_pFilteredMatrix[i*m_mDim.m_x+j]->v();
			if(cellDist > distMin)
			{
				distMin = cellDist;
				posMin.m_x = j;
				posMin.m_y = i;
			}
		}
	}

	if(pPos)
	{
		*pPos = posMin;
	}

	double rangeMin, rangeMax;
	m_pStream->getRange(&rangeMin, &rangeMax);
	distMin = rangeMax-(distMin/255.0)*(rangeMax-rangeMin);

	return distMin;
}

vInt2 _Obstacle::matrixDim(void)
{
	return m_mDim;
}

bool _Obstacle::draw(void)
{
	CHECK_F(!this->_ThreadBase::draw());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->getCMat();
	CHECK_F(pMat->empty());
	NULL_F(m_pStream);
	CHECK_F(m_pMatrix->empty());

	Mat mM = *m_pMatrix->getCMat();
	CHECK_F(mM.empty());

    Mat filterM = Mat::zeros(Size(m_mDim.m_x,m_mDim.m_y), CV_8UC1);

	int i,j;
	for(i=0;i<m_mDim.m_y;i++)
	{
		for(j=0;j<m_mDim.m_x;j++)
		{
			filterM.at<uchar>(i,j) = (uchar)(m_pFilteredMatrix[i*m_mDim.m_x+j]->v());
		}
	}

	Mat mA;
    mA = Mat::zeros(Size(m_mDim.m_x,m_mDim.m_y), CV_8UC1);
	vector<Mat> channels;
    channels.push_back(mA);
    channels.push_back(mA);
    channels.push_back(filterM);
    Mat mB;
    cv::merge(channels, mB);

	cv::resize(mB, mA, Size(pMat->cols, pMat->rows),0,0,INTER_NEAREST);
	cv::addWeighted(*pMat, 1.0, mA, m_dBlend, 0.0, *pMat);

	return true;
}

}

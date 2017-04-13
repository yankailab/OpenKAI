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
	m_pZed = NULL;
	m_pMatrix = NULL;
	m_nFilter = 0;
	m_dBlend = 0.5;
	m_mDim.x = 10;
	m_mDim.y = 10;
	m_fRoi.x = 0.0;
	m_fRoi.y = 0.0;
	m_fRoi.z = 1.0;
	m_fRoi.w = 1.0;
	m_iRoi.init();

}

_Obstacle::~_Obstacle()
{
	DEL(m_pMatrix);

	for (int i = 0; i < m_nFilter; i++)
		DEL(m_pFilteredMatrix[i]);
}

bool _Obstacle::init(void* pKiss)
{
	IF_F(!_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	string presetDir = "";
	F_INFO(pK->root()->o("APP")->v("presetDir", &presetDir));
	F_INFO(pK->v("dBlend", &m_dBlend));
	F_INFO(pK->v("matrixW", &m_mDim.x));
	F_INFO(pK->v("matrixH", &m_mDim.y));
	F_INFO(pK->v("orientation", (int*)&m_orientation));

	F_INFO(pK->v("roiL", &m_fRoi.x));
	F_INFO(pK->v("roiT", &m_fRoi.y));
	F_INFO(pK->v("roiR", &m_fRoi.z));
	F_INFO(pK->v("roiB", &m_fRoi.w));

	m_iRoi.x = m_fRoi.x * m_mDim.x;
	m_iRoi.y = m_fRoi.y * m_mDim.y;
	m_iRoi.z = m_fRoi.z * m_mDim.x;
	m_iRoi.w = m_fRoi.w * m_mDim.y;

	if(m_iRoi.x < 0)m_iRoi.x=0;
	if(m_iRoi.y < 0)m_iRoi.y=0;
	if(m_iRoi.z >= m_mDim.x)m_iRoi.z=m_mDim.x-1;
	if(m_iRoi.w >= m_mDim.y)m_iRoi.w=m_mDim.y-1;

	m_nFilter = m_mDim.area();
	IF_F(m_nFilter >= N_FILTER);

	Kiss* pCC;
	int i;

	//filter
	pCC = pK->o("medianFilter");
	IF_F(pCC->empty());

	for (i = 0; i < m_nFilter; i++)
	{
		m_pFilteredMatrix[i] = new Median();
		IF_F(!m_pFilteredMatrix[i]->init(pCC));
	}

	m_pMatrix = new Frame();

	return true;
}

bool _Obstacle::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("_ZED", &iName));
	m_pZed = (_ZED*) (pK->root()->getChildInstByName(&iName));

	IF_F(!m_pZed);
	m_range = m_pZed->range();

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
	NULL_(m_pZed);
	IF_(!m_pZed->isOpened());

	Frame* pDepth = m_pZed->depth();
	NULL_(pDepth);
	IF_(pDepth->empty());

	m_pMatrix->getResizedOf(m_pZed->depth(), m_mDim.x, m_mDim.y);
	Mat* pM = m_pMatrix->getCMat();

	int i,j;
	for(i=0;i<m_mDim.y;i++)
	{
		for(j=0;j<m_mDim.x;j++)
		{
			m_pFilteredMatrix[i*m_mDim.x+j]->input((double)pM->at<float>(i,j));
		}
	}
}

double _Obstacle::d(void)
{
	if(!m_pZed)return -1.0;

	double dMin = m_range.y;
	int i,j;
	for(i=m_iRoi.y;i<m_iRoi.w;i++)
	{
		for(j=m_iRoi.x;j<m_iRoi.z;j++)
		{
			double dCell = m_pFilteredMatrix[i*m_mDim.x+j]->v();
			IF_CONT(dCell < m_range.x);
			IF_CONT(dCell > m_range.y);
			IF_CONT(dCell > dMin);

			dMin = dCell;
			m_posMin.x = j;
			m_posMin.y = i;
		}
	}

	return dMin;
}

vInt2 _Obstacle::matrixDim(void)
{
	return m_mDim;
}

DISTANCE_SENSOR_TYPE _Obstacle::type(void)
{
	return dsZED;
}

bool _Obstacle::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->getCMat();
	IF_F(pMat->empty());
	NULL_F(m_pZed);
	IF_F(m_pMatrix->empty());

	Mat mM = *m_pMatrix->getCMat();
	IF_F(mM.empty());

	double normD;
	double baseD = 255.0/(m_range.y - m_range.x);

    Mat filterM = Mat::zeros(Size(m_mDim.x,m_mDim.y), CV_8UC1);
	int i,j;
	for(i=0;i<m_mDim.y;i++)
	{
		for(j=0;j<m_mDim.x;j++)
		{
			normD = m_pFilteredMatrix[i*m_mDim.x+j]->v();
			normD = (normD - m_range.x) * baseD;
			filterM.at<uchar>(i,j) = 255 - (uchar)normD;
		}
	}

	Mat mA;
    mA = Mat::zeros(Size(m_mDim.x,m_mDim.y), CV_8UC1);
	vector<Mat> channels;
    channels.push_back(mA);
    channels.push_back(mA);
    channels.push_back(filterM);
    Mat mB;
    cv::merge(channels, mB);
	cv::resize(mB, mA, Size(pMat->cols, pMat->rows),0,0,INTER_NEAREST);
	cv::addWeighted(*pMat, 1.0, mA, m_dBlend, 0.0, *pMat);

	Rect r;
	r.x = m_fRoi.x * ((double)pMat->cols);
	r.y = m_fRoi.y * ((double)pMat->rows);
	r.width = m_fRoi.z * ((double)pMat->cols) - r.x;
	r.height = m_fRoi.w * ((double)pMat->rows) - r.y;
	rectangle(*pMat, r, Scalar(0,255,255), 1);

	circle(*pMat, Point((m_posMin.x+0.5)*(pMat->cols/m_mDim.x), (m_posMin.y+0.5)*(pMat->rows/m_mDim.y)),
			0.000025*pMat->cols*pMat->rows,
			Scalar(0, 255, 255), 2);


	return true;
}

}

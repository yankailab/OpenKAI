/*
 * _DepthVisionBase.cpp
 *
 *  Created on: Apr 6, 2018
 *      Author: yankai
 */

#include "_DepthVisionBase.h"

namespace kai
{

_DepthVisionBase::_DepthVisionBase()
{
	m_pDepthWin = NULL;
	m_range.x = 0.0;
	m_range.y = 10.0;
	m_wD = 1280;
	m_hD = 720;
	m_dShowAlpha = 0.5;
	m_bShowRawDepth = false;
	m_dShowRawBase = 1.0/15.0;

	m_pFilterMatrix = NULL;
	m_nFilter = 0;
	m_mDim.x = 10;
	m_mDim.y = 10;
}

_DepthVisionBase::~_DepthVisionBase()
{
	DEL(m_pFilterMatrix);
}

bool _DepthVisionBase::init(void* pKiss)
{
	IF_F(!this->_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	KISSm(pK,wD);
	KISSm(pK,hD);
	KISSm(pK,dShowAlpha);
	KISSm(pK,bShowRawDepth);
	KISSm(pK,dShowRawBase);

	F_INFO(pK->v("rFrom", &m_range.x));
	F_INFO(pK->v("rTo", &m_range.y));

	F_INFO(pK->v("mW", &m_mDim.x));
	F_INFO(pK->v("mH", &m_mDim.y));
	m_nFilter = m_mDim.area();
	m_pFilterMatrix = new Median[m_nFilter];

	int nMed=0;
	F_INFO(pK->v("nMed", &nMed));
	for (int i = 0; i < m_nFilter; i++)
	{
		m_pFilterMatrix[i].init(nMed,0);
	}

	return true;
}

bool _DepthVisionBase::link(void)
{
	IF_F(!this->_VisionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("depthWindow", &iName));
	m_pDepthWin = (Window*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void _DepthVisionBase::postProcessDepth(void)
{
	if (m_bCalibration)
		m_fDepth = m_fDepth.remap();

	if (m_bGimbal)
		m_fDepth = m_fDepth.warpAffine(m_rotRoll);

	if (m_bFlip)
		m_fDepth = m_fDepth.flip(-1);

	if (m_bCrop)
		m_fDepth = m_fDepth.crop(m_cropBB);
}

void _DepthVisionBase::updateFilter(void)
{
	IF_(m_fDepth.bEmpty());

	m_fMatrixFrame = m_fDepth.resize(m_mDim.x, m_mDim.y);
	Mat* pM = m_fMatrixFrame.m();
	IF_(pM->empty());

	int i,j;
	for(i=0;i<m_mDim.y;i++)
	{
		for(j=0;j<m_mDim.x;j++)
		{
			m_pFilterMatrix[i*m_mDim.x+j].input((double)pM->at<float>(i,j));
		}
	}
}

double _DepthVisionBase::d(vDouble4* pROI, vInt2* pPos)
{
	if(!pROI)return -1.0;

	vInt4 iR;
	iR.x = pROI->x * m_mDim.x;
	iR.y = pROI->y * m_mDim.y;
	iR.z = pROI->z * m_mDim.x;
	iR.w = pROI->w * m_mDim.y;

	if (iR.x < 0)
		iR.x = 0;
	if (iR.y < 0)
		iR.y = 0;
	if (iR.z >= m_mDim.x)
		iR.z = m_mDim.x - 1;
	if (iR.w >= m_mDim.y)
		iR.w = m_mDim.y - 1;

	return d(&iR, pPos);
}

double _DepthVisionBase::d(vInt4* pROI, vInt2* pPos)
{
	if(!pROI)return -1.0;

	double dMin = m_range.y;
	for(int i=pROI->y; i<pROI->w; i++)
	{
		for(int j=pROI->x; j<pROI->z; j++)
		{
			double dCell = m_pFilterMatrix[i*m_mDim.x+j].v();
			IF_CONT(dCell < m_range.x);
			IF_CONT(dCell > m_range.y);
			IF_CONT(dCell > dMin);

			dMin = dCell;
			if(pPos)
			{
				pPos->x = j;
				pPos->y = i;
			}
		}
	}

	return dMin;
}

vInt2 _DepthVisionBase::matrixDim(void)
{
	return m_mDim;
}

Frame* _DepthVisionBase::Depth(void)
{
	return &m_fDepth;
}

vDouble2 _DepthVisionBase::range(void)
{
	return m_range;
}

bool _DepthVisionBase::draw(void)
{
	IF_F(!this->_VisionBase::draw());
	Frame* pFrame = ((Window*)m_pWindow)->getFrame();

	if(m_bShowRawDepth)
	{
		IF_F(m_depthShow.bEmpty());
	}
	else
	{
		IF_F(pFrame->bEmpty());

		Mat mF = Mat::zeros(m_mDim.x, m_mDim.y, CV_8UC3);

		int i,j;
		for(i=0; i<m_mDim.y; i++)
		{
			for(j=0; j<m_mDim.x; j++)
			{
				mF.at<Vec3b>(i,j)[2] = 255 * (1.0 - (m_pFilterMatrix[i*m_mDim.x+j].v() * m_dShowRawBase));
			}
		}

		Frame fF;
		fF.copy(mF);
		Mat* pM = pFrame->m();
		m_depthShow = fF.resize(pM->cols, pM->rows);
	}

	if(m_pDepthWin)
	{
		m_pDepthWin->getFrame()->copy(*m_depthShow.m());
	}
	else if(m_pWindow)
	{
		Mat mW;
		Mat* pM = pFrame->m();
		Mat* pD = m_depthShow.m();
		IF_F(pM->cols != pD->cols || pM->rows != pD->rows);
		cv::addWeighted(*pD, m_dShowAlpha, *pM, 1.0, 0, mW);
		pFrame->copy(mW);
	}

	return true;
}

}

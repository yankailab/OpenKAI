/*
 * CameraOpticalFlow.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "_DenseFlowTracker.h"

#include "../Base/common.h"
#include "stdio.h"
#include "../Base/cvplatform.h"

namespace kai
{

_DenseFlowTracker::_DenseFlowTracker()
{
	_ThreadBase();

	m_flowMax = 1e9;
	m_flowAvr = 0.0;

	m_pDF = NULL;

}

_DenseFlowTracker::~_DenseFlowTracker()
{
}

bool _DenseFlowTracker::init(JSON* pJson, string camName)
{
//	CHECK_INFO(pJson->getVal("DENSEFLOW_"+camName+"_WIDTH", &m_size.width));
//	CHECK_INFO(pJson->getVal("DENSEFLOW_"+camName+"_HEIGHT", &m_size.height));

	m_ROI.width = 100;
	m_ROI.height = 100;
	m_ROI.x = 0;//m_size.width*0.5 - m_ROI.width*0.5;
	m_ROI.y = 0;//m_size.height*0.5 - m_ROI.height*0.5;

	m_tSleep = TRD_INTERVAL_DENSEFLOWTRACKER;
	return true;
}


bool _DenseFlowTracker::start(void)
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

void _DenseFlowTracker::update(void)
{
	while (m_bThreadON)
	{
		this->updateTime();

		track();

		if(m_tSleep>0)
		{
			//sleepThread can be woke up by this->wakeupThread()
			this->sleepThread(0, m_tSleep);
		}
	}

}

void _DenseFlowTracker::track(void)
{
	int i, j;
	Point2f vFlow;
	Mat flowMat;
	double flowDist;
	double flowTot, newMax;

	if(m_pDF == NULL)return;
	flowMat = m_pDF->m_cFlowMat;
	if(flowMat.empty())return;

	if(flowMat.size() != m_Mat.size())
	{
		m_Mat = Mat(flowMat.size(), CV_8UC1);
	}

	flowTot = 0.0;
	newMax = 0.0;

	m_flowMax = 15;
	double fInterval = m_flowMax/256;
	uchar segCol;

	for (i = 0; i < flowMat.rows; i++)
	{
		for (j = 0; j < flowMat.cols; j++)
		{
			vFlow = flowMat.at<cv::Point2f>(i, j);
			flowDist = (abs(vFlow.x) + abs(vFlow.y));
//			if(flowDist>1e3)flowDist=0.0;
			flowDist = min(m_flowMax, flowDist);

			segCol = (uchar)(flowDist / fInterval);

			m_Mat.at<uchar>(i,j) = segCol;//(uchar)(flowDist - m_flowAvr);

			flowTot += flowDist;
			newMax = max(newMax,flowDist);
		}
	}

	m_flowAvr = flowTot / (flowMat.rows * flowMat.cols);
	m_flowMax = newMax;

}


} /* namespace kai */

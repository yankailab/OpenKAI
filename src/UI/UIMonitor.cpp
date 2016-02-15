/*
 * CamMonitor.cpp
 *
 *  Created on: Aug 23, 2015
 *      Author: yankai
 */

#include "UIMonitor.h"

namespace kai
{

UIMonitor::UIMonitor()
{
	m_windowName = "default";
	m_width = 0;
	m_height = 0;
	m_numFrame = 0;

}

UIMonitor::~UIMonitor()
{
	// TODO Auto-generated destructor stub
}

bool UIMonitor::init(string name, JSON* pJson)
{
	if(name.empty())return false;
	if(pJson==NULL)return false;

	CHECK_FATAL(pJson->getVal("UI_MAIN_WIDTH", &m_width));
	CHECK_FATAL(pJson->getVal("UI_MAIN_HEIGHT", &m_height));

	m_windowName = name;
	m_numFrame = 0;

	m_showFrame = UMat(m_height,m_width,CV_8UC3,cv::Scalar(0,0,0));

	return true;
}

bool UIMonitor::addFullFrame(CamFrame* pFrame)
{
	MONITOR_FRAME* pMFrame;

	if(!pFrame)return false;
	if(m_numFrame >= NUM_MONITOR_FRAME)return false;

	pMFrame = &m_pFrame[m_numFrame];
	pMFrame->m_pFrame = pFrame;
	pMFrame->m_x = 0;
	pMFrame->m_y = 0;
	pMFrame->m_w = m_width;
	pMFrame->m_h = m_height;
	m_numFrame++;

	return true;
}

bool UIMonitor::addFrame(CamFrame* pFrame, int x, int y, int w, int h)
{
	MONITOR_FRAME* pMFrame;

	if(!pFrame)return false;
	if(x<0 || y<0)return false;
	if(h<=0 || w<=0)return false;
	if(m_numFrame >= NUM_MONITOR_FRAME)return false;

	pMFrame = &m_pFrame[m_numFrame];
	pMFrame->m_pFrame = pFrame;
	pMFrame->m_x = x;
	pMFrame->m_y = y;
	pMFrame->m_w = w;
	pMFrame->m_h = h;
	m_numFrame++;

	return true;
}

bool UIMonitor::addFrame(MONITOR_FRAME* pMFrame)
{
	if(!pMFrame)return false;
	if(pMFrame->m_x<0 || pMFrame->m_x<0)return false;
	if(pMFrame->m_h<=0 || pMFrame->m_w<=0)return false;
	if(m_numFrame >= NUM_MONITOR_FRAME)return false;

	m_pFrame[m_numFrame] = *pMFrame;
	m_numFrame++;

	return true;
}

bool UIMonitor::updateFrame(MONITOR_FRAME* pMFrame)
{
	if(!pMFrame)return false;
	if(pMFrame->m_x<0 || pMFrame->m_x<0)return false;
	if(pMFrame->m_h<=0 || pMFrame->m_w<=0)return false;

	int i;

	for(i=0;i<m_numFrame;i++)
	{
		if(m_pFrame[i].m_pFrame == pMFrame->m_pFrame)
		{
			m_pFrame[i] = *pMFrame;
			return true;
		}
	}

	return false;

}

void UIMonitor::show(void)
{
	int i;
	CamFrame* pFrame;
	MONITOR_FRAME* pMFrame;

	if(m_numFrame == 0)return;

	m_showFrame = UMat(m_height,m_width,CV_8UC3,cv::Scalar(0,0,0));

	for(i=0;i<m_numFrame;i++)
	{
		//Ignore if empty
		pMFrame = &m_pFrame[i];
		pFrame = pMFrame->m_pFrame;
		if(pFrame->getCurrent()->empty())continue;

		//Resize and download
//		pFrame->getResized(pMFrame->m_w, pMFrame->m_h, &m_camFrame);
		m_camFrame.getResizedOf(pFrame, pMFrame->m_w, pMFrame->m_h);

		//Convert color mode if necesary
		if(m_camFrame.getCurrent()->type() != m_showFrame.type())
		{
			m_camFrame2.get8UC3Of(&m_camFrame);
			m_camFrame2.getCurrent()->download(m_frame);
		}
		else
		{
			m_camFrame.getCurrent()->download(m_frame);
		}

		//Copy to show frame
		m_frame.copyTo(m_showFrame(cv::Rect(pMFrame->m_x, pMFrame->m_y,pMFrame->m_w,pMFrame->m_h)));
	}

	imshow(m_windowName,m_showFrame);
}

void UIMonitor::removeAll(void)
{
	m_numFrame = 0;
}

} /* namespace kai */

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

bool UIMonitor::init(string name, int width, int height)
{
	if(name.empty())return false;
	if(width==0 || height==0)return false;

	m_windowName = name;
	m_width = width;
	m_height = height;
	m_numFrame = 0;

	m_showFrame = Mat(height,width,CV_8UC3,cv::Scalar(0,0,0));
	m_camFrame.init();

	return true;
}

bool UIMonitor::addFrame(CamFrame* pFrame, int x, int y, int w, int h)
{
	MONITOR_FRAME* pMFrame;

	if(!pFrame)return false;
	if(x<0 || y<0)return false;
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

void UIMonitor::show(void)
{
	int i;
	CamFrame* pFrame;
	MONITOR_FRAME* pMFrame;

	if(m_numFrame == 0)return;

	for(i=0;i<m_numFrame;i++)
	{
		//Ignore if empty
		pMFrame = &m_pFrame[i];
		pFrame = pMFrame->m_pFrame;
		if(pFrame->getCurrentFrame()->empty())continue;

		//Resize and download
		pFrame->getResized(pMFrame->m_w, pMFrame->m_h, &m_camFrame);

		//Convert color mode if necesary
		if(m_camFrame.m_pNext->type()!=m_showFrame.type())
		{
			m_camFrame.get8UC3(&m_camFrame2);
			m_camFrame2.getCurrentFrame()->download(m_frame);
		}
		else
		{
			m_camFrame.getCurrentFrame()->download(m_frame);
		}

		//Copy to show frame
		m_frame.copyTo(m_showFrame(cv::Rect(pMFrame->m_x, pMFrame->m_y,pMFrame->m_w,pMFrame->m_h)));
	}

	imshow(m_windowName,m_showFrame);
}

} /* namespace kai */

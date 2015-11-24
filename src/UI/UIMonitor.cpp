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
	// TODO Auto-generated constructor stub

}

UIMonitor::~UIMonitor()
{
	// TODO Auto-generated destructor stub
}


void UIMonitor::setWindowName(string name)
{
	m_windowName = name;
}

bool UIMonitor::init(void)
{

	return true;
}

void UIMonitor::addFrame(CamFrame* pFrame, int zIndex, double alpha)
{
//	m_mat = pFrame->m_uFrame;
//	pFrame->m_uFrame.copyTo(m_mat);
	pFrame->m_pNext->download(m_mat);
}

void UIMonitor::addMarkerDetect(CamFrame* pFrame, int zIndex, double alpha)
{

}

void UIMonitor::addDenseFlow(CamFrame* pFrame, int zIndex, double alpha)
{

}

void UIMonitor::addSparseFlow(CamFrame* pFrame, int zIndex, double alpha)
{

}

void UIMonitor::addObjectDetect(CamFrame* pFrame, int zIndex, double alpha)
{

}

void UIMonitor::show(void)
{
	imshow(m_windowName,m_mat);
}

} /* namespace kai */

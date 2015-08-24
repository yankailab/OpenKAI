/*
 * CamMonitor.cpp
 *
 *  Created on: Aug 23, 2015
 *      Author: yankai
 */

#include "CamMonitor.h"

namespace kai
{

CamMonitor::CamMonitor()
{
	// TODO Auto-generated constructor stub

}

CamMonitor::~CamMonitor()
{
	// TODO Auto-generated destructor stub
}


void CamMonitor::setWindowName(string name)
{
	m_windowName = name;
}

bool CamMonitor::init(void)
{

	return true;
}

void CamMonitor::addFrame(CamFrame* pFrame, int zIndex, double alpha)
{
	pFrame->m_uFrame.copyTo(m_mat);
}
void CamMonitor::addMarkerDetect(CamFrame* pFrame, int zIndex, double alpha)
{

}

void CamMonitor::addDenseFlow(CamFrame* pFrame, int zIndex, double alpha)
{

}

void CamMonitor::addSparseFlow(CamFrame* pFrame, int zIndex, double alpha)
{

}

void CamMonitor::addObjectDetect(CamFrame* pFrame, int zIndex, double alpha)
{

}

void CamMonitor::show(void)
{
	imshow(m_windowName,m_mat);
}

} /* namespace kai */

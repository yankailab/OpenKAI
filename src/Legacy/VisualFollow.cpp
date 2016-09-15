/*
#include "VisualFollow.h"

namespace kai
{

void VisualFollow::handleMouse(int event, int x, int y, int flags)
{
	Rect2d roi;
	int ROIhalf;

	if(m_ROImode == MODE_ASSIST)
	{
		switch (event)
		{
		case EVENT_LBUTTONDOWN:
			if(x > 1820)
			{
				if(y < 100)
				{
					//Clear ROI
					m_pROITracker->tracking(false);
					m_bSelect = false;
					return;
				}
				else if(y < 200)
				{
					//Magnify the ROI size
					if(m_ROIsize < m_ROIsizeTo)
					{
						m_ROIsize += 10;
						if(m_pROITracker->m_bTracking)
						{
							roi.x = m_pROITracker->m_ROI.x + m_pROITracker->m_ROI.width/2;
							roi.y = m_pROITracker->m_ROI.y + m_pROITracker->m_ROI.height/2;
							ROIhalf = m_ROIsize/2;

							m_ROI.m_x = roi.x - ROIhalf;
							m_ROI.m_y = roi.y - ROIhalf;
							m_ROI.m_z = roi.x + ROIhalf;
							m_ROI.m_w = roi.y + ROIhalf;
							roi = getROI(m_ROI);
							m_pROITracker->setROI(roi);
							m_bSelect = true;
						}
					}
					m_bSelect = false;
					return;
				}
				else if(y < 300)
				{
					//Shrink the ROI size
					if(m_ROIsize > m_ROIsizeFrom)
					{
						m_ROIsize -= 10;
						if(m_pROITracker->m_bTracking)
						{
							roi.x = m_pROITracker->m_ROI.x + m_pROITracker->m_ROI.width/2;
							roi.y = m_pROITracker->m_ROI.y + m_pROITracker->m_ROI.height/2;
							ROIhalf = m_ROIsize/2;

							m_ROI.m_x = roi.x - ROIhalf;
							m_ROI.m_y = roi.y - ROIhalf;
							m_ROI.m_z = roi.x + ROIhalf;
							m_ROI.m_w = roi.y + ROIhalf;
							roi = getROI(m_ROI);
							m_pROITracker->setROI(roi);
							m_bSelect = true;
						}
					}
					m_bSelect = false;
					return;
				}
				else if(y > m_btnMode)
				{
					m_ROI.m_x = 0;
					m_ROI.m_y = 0;
					m_ROI.m_z = 0;
					m_ROI.m_w = 0;
					m_pROITracker->tracking(false);
					m_bSelect = false;
					m_ROImode = MODE_RECTDRAW;
					return;
				}
			}

			ROIhalf = m_ROIsize/2;

			m_ROI.m_x = x - ROIhalf;
			m_ROI.m_y = y - ROIhalf;
			m_ROI.m_z = x + ROIhalf;
			m_ROI.m_w = y + ROIhalf;
			roi = getROI(m_ROI);
			m_pROITracker->setROI(roi);
			m_pROITracker->tracking(true);
			m_bSelect = true;
			break;
		case EVENT_MOUSEMOVE:
			if(m_bSelect)
			{
				ROIhalf = m_ROIsize/2;
				m_ROI.m_x = x - ROIhalf;
				m_ROI.m_y = y - ROIhalf;
				m_ROI.m_z = x + ROIhalf;
				m_ROI.m_w = y + ROIhalf;
				roi = getROI(m_ROI);
				m_pROITracker->setROI(roi);
				m_pROITracker->tracking(true);
			}
			break;
		case EVENT_LBUTTONUP:
			m_bSelect = false;
			break;
		case EVENT_RBUTTONDOWN:
			break;
		default:
			break;
		}

	}
	else if(m_ROImode == MODE_RECTDRAW)
	{
		switch (event)
		{
		case EVENT_LBUTTONDOWN:
			if(x > 1820)
			{
				if(y > m_btnMode)
				{
					m_ROI.m_x = 0;
					m_ROI.m_y = 0;
					m_ROI.m_z = 0;
					m_ROI.m_w = 0;
					m_pROITracker->tracking(false);
					m_bSelect = false;
					m_ROImode = MODE_ASSIST;
					return;
				}
			}

			m_pROITracker->tracking(false);
			m_ROI.m_x = x;
			m_ROI.m_y = y;
			m_ROI.m_z = x;
			m_ROI.m_w = y;
			m_bSelect = true;
			break;
		case EVENT_MOUSEMOVE:
			if(m_bSelect)
			{
				m_ROI.m_z = x;
				m_ROI.m_w = y;
			}
			break;
		case EVENT_LBUTTONUP:
			roi = getROI(m_ROI);
			if(roi.width<m_minROI || roi.height<m_minROI)
			{
				m_ROI.m_x = 0;
				m_ROI.m_y = 0;
				m_ROI.m_z = 0;
				m_ROI.m_w = 0;
			}
			else
			{
				m_pROITracker->setROI(roi);
				m_pROITracker->tracking(true);
			}
			m_bSelect = false;
			break;
		case EVENT_RBUTTONDOWN:
			break;
		default:
			break;
		}
	}

}

Rect2d VisualFollow::getROI(iVector4 mouseROI)
{
	Rect2d roi;

	roi.x = min(mouseROI.m_x,mouseROI.m_z);
	roi.y = min(mouseROI.m_y,mouseROI.m_w);
	roi.width = abs(mouseROI.m_z - mouseROI.m_x);
	roi.height = abs(mouseROI.m_w - mouseROI.m_y);

	return roi;
}


}
*/

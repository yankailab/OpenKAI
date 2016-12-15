#include "APMcopter_landing.h"

namespace kai
{

APMcopter_landing::APMcopter_landing()
{
	ActionBase();

	m_pROITracker = NULL;
	m_pMD = NULL;
	m_pAT = NULL;
	m_numATagsLandingTarget = 0;

	m_pAPM = NULL;
}

APMcopter_landing::~APMcopter_landing()
{
}

bool APMcopter_landing::init(void* pKiss)
{
	CHECK_F(this->ActionBase::init(pKiss)==false);
	Kiss* pK = (Kiss*)pKiss;

	m_landingTarget.m_angleX = 0;
	m_landingTarget.m_angleY = 0;
	m_landingTarget.m_orientX = 1.0;
	m_landingTarget.m_orientY = 1.0;
	m_landingTarget.m_ROIstarted = 0;
	m_landingTarget.m_ROItimeLimit = 0;

	F_INFO(pK->v("orientationX", &m_landingTarget.m_orientX));
	F_INFO(pK->v("orientationY", &m_landingTarget.m_orientY));
	F_INFO(pK->v("roiTimeLimit", &m_landingTarget.m_ROItimeLimit));

	Kiss* pCC = pK->o("AprilTags");
	if(pCC->empty())return false;

	F_INFO(pCC->v("num", &m_numATagsLandingTarget));
	for(int i=0; i<m_numATagsLandingTarget; i++)
	{
		F_ERROR_F(pCC->v("tag"+i2str(i), &m_pATagsLandingTarget[i]));
	}

	pK->m_pInst = this;

	return true;
}

bool APMcopter_landing::link(void)
{
	CHECK_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*)m_pKiss;

	string iName = "";

	F_INFO(pK->v("APMcopter_base", &iName));
	m_pAPM = (APMcopter_base*) (pK->root()->getChildInstByName(&iName));

	F_ERROR_F(pK->v("ROItracker", &iName));
	m_pROITracker = (_ROITracker*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void APMcopter_landing::update(void)
{
	this->ActionBase::update();

	NULL_(m_pAPM);
	CHECK_(m_pAM->getCurrentStateIdx() != m_iActiveState);

	landingAtAprilTags();

}

void APMcopter_landing::landingAtAprilTags(void)
{
	NULL_(m_pAT);
	NULL_(m_pAT->m_pStream);
	NULL_(m_pAPM->m_pMavlink);

	int i;
	int tTag;
	CamBase* pCam = m_pAT->m_pStream->getCameraInput();

	for(i=0;i<m_numATagsLandingTarget;i++)
	{
		tTag = m_pATagsLandingTarget[i];
		if (m_pAT->getTags(tTag, m_pATags)<=0)
		{
			m_pATags[0].m_frameID = 0;
			continue;
		}

		APRIL_TAG* pTag = &m_pATags[0];

		//Change position to angles
		m_landingTarget.m_angleX = ((pTag->m_tag.cxy.x - pCam->m_centerH) / pCam->m_width) * pCam->m_angleH * DEG_RADIAN * m_landingTarget.m_orientX;
		m_landingTarget.m_angleY = ((pTag->m_tag.cxy.y - pCam->m_centerV) / pCam->m_height) * pCam->m_angleV * DEG_RADIAN * m_landingTarget.m_orientY;

		//Send Mavlink command
		m_pAPM->m_pMavlink->landing_target(MAV_DATA_STREAM_ALL, MAV_FRAME_BODY_NED,
				m_landingTarget.m_angleX, m_landingTarget.m_angleY, 0, 0, 0);

		return;
	}
}

void APMcopter_landing::landingAtBullseye(void)
{
	CamBase* pCamInput;
	vDouble3 markerCenter;

	if (m_pMD == NULL)
		return;
	if (m_pROITracker == NULL)
		return;

	if (m_pMD->getCircleCenter(&markerCenter))
	{
		//Update Tracker
		Rect roi;
		roi.x = markerCenter.m_x - markerCenter.m_z;
		roi.y = markerCenter.m_y - markerCenter.m_z;
		roi.width = markerCenter.m_z * 2;
		roi.height = markerCenter.m_z * 2;
		m_pROITracker->setROI(roi);
		m_pROITracker->tracking(true);

		m_landingTarget.m_ROIstarted = 0;
	}
	else
	{
		if (!m_pROITracker->m_bTracking)
			return;

		uint64_t timeNow = get_time_usec();

		//ROI tracker is already in use
		if (m_landingTarget.m_ROIstarted > 0)
		{
			//Disable sending landing target if the marker is not seen for a certain time
			if (timeNow - m_landingTarget.m_ROIstarted
					> m_landingTarget.m_ROItimeLimit)
			{
				m_pROITracker->tracking(false);
				return;
			}
		}
		else
		{
			//Start to use ROI tracker if marker is not detected
			m_landingTarget.m_ROIstarted = timeNow;
		}

		markerCenter.m_x = m_pROITracker->m_ROI.x
				+ m_pROITracker->m_ROI.width * 0.5;
		markerCenter.m_y = m_pROITracker->m_ROI.y
				+ m_pROITracker->m_ROI.height * 0.5;
	}

	pCamInput = m_pMD->m_pStream->getCameraInput();

	//Change position to angles
	m_landingTarget.m_angleX = ((markerCenter.m_x - pCamInput->m_centerH)
			/ pCamInput->m_width) * pCamInput->m_angleH * DEG_RADIAN
			* m_landingTarget.m_orientX;
	m_landingTarget.m_angleY = ((markerCenter.m_y - pCamInput->m_centerV)
			/ pCamInput->m_height) * pCamInput->m_angleV * DEG_RADIAN
			* m_landingTarget.m_orientY;

	NULL_(m_pAPM->m_pMavlink);

	//Send Mavlink command
	m_pAPM->m_pMavlink->landing_target(MAV_DATA_STREAM_ALL, MAV_FRAME_BODY_NED,
			m_landingTarget.m_angleX, m_landingTarget.m_angleY, 0, 0, 0);

}

bool APMcopter_landing::draw(void)
{
	CHECK_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	if(m_pAT)
	{
		APRIL_TAG* pTag = &m_pATags[0];
		if(pTag->m_frameID>0)
		{
			circle(*pMat, pTag->m_tag.cxy, 10, Scalar(0, 0, 255), 5);
			putText(*pMat,
					"Landing_Target: (" + f2str(m_landingTarget.m_angleX) + " , "
							+ f2str(m_landingTarget.m_angleY) + ")",
					*pWin->getTextPos(), FONT_HERSHEY_SIMPLEX, 0.5,
					Scalar(0, 255, 0), 1);

			pWin->lineNext();
		}
	}

	if(m_pMD)
	{
		vDouble3 markerCenter;
		markerCenter.m_x = 0;
		markerCenter.m_y = 0;
		markerCenter.m_z = 0;

		if (m_pMD->getCircleCenter(&markerCenter))
		{
			circle(*pMat, Point(markerCenter.m_x, markerCenter.m_y),
					markerCenter.m_z, Scalar(0, 0, 255), 5);
		}
		else if (m_pROITracker->m_bTracking)
		{
			rectangle(*pMat, Point(m_pROITracker->m_ROI.x, m_pROITracker->m_ROI.y),
					Point(m_pROITracker->m_ROI.x + m_pROITracker->m_ROI.width,
							m_pROITracker->m_ROI.y + m_pROITracker->m_ROI.height),
					Scalar(0, 0, 255), 3);
		}

		putText(*pMat,
				"Landing_Target: (" + f2str(m_landingTarget.m_angleX) + " , "
						+ f2str(m_landingTarget.m_angleY) + ")",
				*pWin->getTextPos(), FONT_HERSHEY_SIMPLEX, 0.5,
				Scalar(0, 255, 0), 1);

		pWin->lineNext();
	}

	return true;
}


}

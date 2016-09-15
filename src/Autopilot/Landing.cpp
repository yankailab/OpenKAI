#include "Landing.h"

namespace kai
{

Landing::Landing()
{
	ActionBase();

	m_pFD = NULL;
	m_pROITracker = NULL;
	m_pMD = NULL;
	m_pAT = NULL;
	m_numATagsLandingTarget = 0;

}

Landing::~Landing()
{
}

bool Landing::init(Config* pConfig, string name)
{
	if (this->ActionBase::init(pConfig,name)==false)
		return false;

	int i;
	Config* pC = pConfig->o(name);
	Config* pCC = pC->o("visualLanding");
	if(pCC->empty())return false;

	m_landingTarget.m_angleX = 0;
	m_landingTarget.m_angleY = 0;
	m_landingTarget.m_orientX = 1.0;
	m_landingTarget.m_orientY = 1.0;
	m_landingTarget.m_ROIstarted = 0;
	m_landingTarget.m_ROItimeLimit = 0;

	F_INFO_(pCC->v("orientationX", &m_landingTarget.m_orientX));
	F_INFO_(pCC->v("orientationY", &m_landingTarget.m_orientY));
	F_INFO_(pCC->v("roiTimeLimit", &m_landingTarget.m_ROItimeLimit));

	pCC = pCC->o("AprilTags");
	if(pCC->empty())return false;

	F_INFO_(pCC->v("num", &m_numATagsLandingTarget));
	for(i=0; i<m_numATagsLandingTarget; i++)
	{
		F_ERROR_F(pCC->v("tag"+i2str(i), &m_pATagsLandingTarget[i]));
	}

	return true;
}

void Landing::update(void)
{
	this->ActionBase::update();

	landingAtAprilTags();

}

void Landing::landingAtAprilTags(void)
{
	if (m_pAT == NULL)
		return;
	if (m_pMavlink == NULL)
		return;
	if (m_pAT->m_pCamStream == NULL)
		return;

	int i;
	int tTag;
	CamBase* pCam = m_pAT->m_pCamStream->getCameraInput();

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
		m_pMavlink->landing_target(MAV_DATA_STREAM_ALL, MAV_FRAME_BODY_NED,
				m_landingTarget.m_angleX, m_landingTarget.m_angleY, 0, 0, 0);

		return;
	}
}

void Landing::landingAtBullseye(void)
{
	CamBase* pCamInput;
	fVec3 markerCenter;

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

	pCamInput = m_pMD->m_pCamStream->getCameraInput();

	//Change position to angles
	m_landingTarget.m_angleX = ((markerCenter.m_x - pCamInput->m_centerH)
			/ pCamInput->m_width) * pCamInput->m_angleH * DEG_RADIAN
			* m_landingTarget.m_orientX;
	m_landingTarget.m_angleY = ((markerCenter.m_y - pCamInput->m_centerV)
			/ pCamInput->m_height) * pCamInput->m_angleV * DEG_RADIAN
			* m_landingTarget.m_orientY;

	if (m_pMavlink == NULL)
		return;

	//Send Mavlink command
	m_pMavlink->landing_target(MAV_DATA_STREAM_ALL, MAV_FRAME_BODY_NED,
			m_landingTarget.m_angleX, m_landingTarget.m_angleY, 0, 0, 0);

}

bool Landing::draw(Frame* pFrame, iVec4* pTextPos)
{
	if (pFrame == NULL)
		return false;

	Mat* pMat = pFrame->getCMat();

	if(m_pAT)
	{
		APRIL_TAG* pTag = &m_pATags[0];
		if(pTag->m_frameID>0)
		{
			circle(*pMat, pTag->m_tag.cxy, 10, Scalar(0, 0, 255), 5);
			putText(*pMat,
					"Landing_Target: (" + f2str(m_landingTarget.m_angleX) + " , "
							+ f2str(m_landingTarget.m_angleY) + ")",
					cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
					Scalar(0, 255, 0), 1);

			pTextPos->m_y += pTextPos->m_w;
		}
	}

	if(m_pMD)
	{
		fVec3 markerCenter;
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
				cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
				Scalar(0, 255, 0), 1);

		pTextPos->m_y += pTextPos->m_w;
	}

	return true;
}


}

#include "_Landing.h"
#include "../Utility/util.h"

namespace kai
{

_Landing::_Landing()
{
	_AutoPilot();

}

_Landing::~_Landing()
{
}

bool _Landing::init(Config* pConfig, string name)
{
	if (this->_AutoPilot::init(pConfig,name)==false)
		return false;

	int i;
	Config* pC = pConfig->o(name);
	Config* pCC;

	return true;
}

bool _Landing::start(void)
{
	//Start thread
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< "Return code: "<< retCode;
		m_bThreadON = false;
		return false;
	}

	LOG(INFO)<< "_Landing.start()";

	return true;
}

void _Landing::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		landingTarget();

		sendHeartbeat();

		this->autoFPSto();

	}
}

void _Landing::landingTarget(void)
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

bool _Landing::draw(Frame* pFrame, iVector4* pTextPos)
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
		fVector3 markerCenter;
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

		putText(*pMat, "Marker FPS: " + i2str(getFrameRate()),
				cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
				Scalar(0, 255, 0), 1);

		pTextPos->m_y += pTextPos->m_w;

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

/*
 * ZED.cpp
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#include "../Vision/_ZED.h"

#ifdef USE_ZED

namespace kai
{

_ZED::_ZED()
{
	m_type = zed;
	m_pZed = NULL;
	m_pzImg = NULL;
	m_pzDepth = NULL;
	m_zedResolution = (int) sl::RESOLUTION_VGA;
	m_zedMinDist = 0.7;
	m_zedMaxDist = 20.0;
	m_zedFPS = DEFAULT_FPS;
	m_zedSenseMode = sl::SENSING_MODE::SENSING_MODE_FILL;
	m_zedDepthMode = sl::DEPTH_MODE_PERFORMANCE;
	m_bZedFlip = false;
	m_zedConfidence = 100;
	m_bZedSpatialMemory = false;
	m_iZedCPUcore = -1;
	m_pDepthWin = NULL;
	m_angleH = 66.7;
	m_angleV = 67.1;
	m_zedTrackState = sl::TRACKING_STATE_OFF;
	m_trackConfidence = 0;
	m_trackCount = 0;
	m_vT.init();
	m_vR.init();
	m_trackDT = 0;
	m_zedL2C = 0.0;
	m_tLastTrack = 0;
	m_zedViewLR = sl::VIEW_LEFT;
}

_ZED::~_ZED()
{
	reset();
}

bool _ZED::init(void* pKiss)
{
	IF_F(!_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK,zedResolution);
	KISSm(pK,zedFPS);
	KISSm(pK,zedDepthMode);
	KISSm(pK,zedMinDist);
	KISSm(pK,zedMaxDist);
	KISSm(pK,bZedFlip);
	KISSm(pK,bZedSpatialMemory);
	KISSm(pK,zedConfidence);
	KISSm(pK,zedViewLR);
	KISSm(pK,iZedCPUcore);
	KISSm(pK,zedSenseMode);

	m_pDepth = new Frame();

	return true;
}

void _ZED::reset()
{
	this->_VisionBase::reset();

	DEL(m_pzImg);
	DEL(m_pzDepth);
	DEL(m_pZed);
}

bool _ZED::link(void)
{
	IF_F(!this->_VisionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("depthWindow", &iName));
	m_pDepthWin = (Window*) (pK->root()->getChildInstByName(&iName));

	return true;
}

bool _ZED::open(void)
{
	sl::InitParameters zedInit;
	zedInit.camera_buffer_count_linux = 4;
	zedInit.camera_disable_self_calib = false;
	zedInit.camera_fps = m_zedFPS;
	zedInit.camera_image_flip = m_bZedFlip;
	zedInit.camera_linux_id = 0;
	zedInit.camera_resolution = (sl::RESOLUTION) m_zedResolution;
	zedInit.coordinate_system = sl::COORDINATE_SYSTEM::COORDINATE_SYSTEM_IMAGE;
	zedInit.coordinate_units = sl::UNIT::UNIT_METER;
	zedInit.depth_minimum_distance = m_zedMinDist;
	zedInit.depth_mode = (sl::DEPTH_MODE) m_zedDepthMode;
	zedInit.sdk_gpu_id = -1;
	zedInit.sdk_verbose = true;

	m_pZed = new sl::Camera();
	sl::ERROR_CODE err = m_pZed->open(zedInit);
	if (err != sl::SUCCESS)
	{
		LOG_E("ZED Error code: " << sl::errorCode2str(err));
		DEL(m_pZed);
		return false;
	}

	m_pZed->setConfidenceThreshold(m_zedConfidence);
	m_pZed->setDepthMaxRangeValue((float) m_zedMaxDist);

	// Set runtime parameters after opening the camera
	m_zedRuntime.sensing_mode = (sl::SENSING_MODE) m_zedSenseMode;
	m_zedRuntime.enable_depth = true;
	m_zedRuntime.enable_point_cloud = false;

	// Create sl and cv Mat to get ZED left image and depth image
	sl::Resolution zedImgSize = m_pZed->getResolution();

	// Initialize color image and depth
	m_width = zedImgSize.width;
	m_height = zedImgSize.height;
	m_centerH = m_width / 2;
	m_centerV = m_height / 2;

	// Best way of sharing sl::Mat and cv::Mat :
	// Create a sl::Mat and then construct a cv::Mat using the ptr to sl::Mat data.
	DEL(m_pzImg);
	DEL(m_pzDepth);

	m_pzImg = new sl::Mat(zedImgSize, sl::MAT_TYPE_8U_C4, sl::MEM_GPU);
	m_gImg = slMat2cvGpuMat(*m_pzImg);
	m_pzDepth = new sl::Mat(zedImgSize, sl::MAT_TYPE_32F_C1, sl::MEM_GPU);
	m_gDepth = slMat2cvGpuMat(*m_pzDepth);

	m_gImg2 = GpuMat(m_gImg.size(), m_gImg.type());
	m_gDepth2 = GpuMat(m_gDepth.size(), m_gDepth.type());

	// Jetson only. Execute the calling thread on 2nd core
	if(m_iZedCPUcore >= 0)
	{
		sl::Camera::sticktoCPUCore(m_iZedCPUcore);
	}

	// Initialize motion tracking parameters
	sl::TrackingParameters zedTracking;
	zedTracking.initial_world_transform = sl::Transform::identity();
	zedTracking.enable_spatial_memory = m_bZedSpatialMemory;
	m_pZed->enableTracking(zedTracking);

	m_zedL2C = (double) (m_pZed->getCameraInformation().calibration_parameters.T.x * 0.5);

	m_bOpen = true;
	return true;
}

GpuMat _ZED::slMat2cvGpuMat(sl::Mat& input)
{
	int h = input.getHeight();
	int w = input.getWidth();

	//convert MAT_TYPE to CV_TYPE
	switch (input.getDataType())
	{
	case sl::MAT_TYPE_32F_C1:
		return GpuMat(h, w, CV_32FC1, input.getPtr<sl::float1>(sl::MEM_GPU));
	case sl::MAT_TYPE_32F_C2:
		return GpuMat(h, w, CV_32FC2, input.getPtr<sl::float1>(sl::MEM_GPU));
	case sl::MAT_TYPE_32F_C3:
		return GpuMat(h, w, CV_32FC3, input.getPtr<sl::float1>(sl::MEM_GPU));
	case sl::MAT_TYPE_32F_C4:
		return GpuMat(h, w, CV_32FC4, input.getPtr<sl::float1>(sl::MEM_GPU));
	case sl::MAT_TYPE_8U_C1:
		return GpuMat(h, w, CV_8UC1, input.getPtr<sl::uchar1>(sl::MEM_GPU));
	case sl::MAT_TYPE_8U_C2:
		return GpuMat(h, w, CV_8UC2, input.getPtr<sl::uchar1>(sl::MEM_GPU));
	case sl::MAT_TYPE_8U_C3:
		return GpuMat(h, w, CV_8UC3, input.getPtr<sl::uchar1>(sl::MEM_GPU));
	case sl::MAT_TYPE_8U_C4:
		return GpuMat(h, w, CV_8UC4, input.getPtr<sl::uchar1>(sl::MEM_GPU));
	default:
		return GpuMat(h, w, CV_8UC4, input.getPtr<sl::uchar1>(sl::MEM_GPU));
	}
}

bool _ZED::start(void)
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

void _ZED::update(void)
{
	while (m_bThreadON)
	{
		if (!m_bOpen)
		{
			if (!open())
			{
				LOG_E("Cannot open ZED");
				this->sleepTime(USEC_1SEC);
				continue;
			}
		}

		this->autoFPSfrom();

		IF_CONT(m_pZed->grab(m_zedRuntime) != sl::SUCCESS);

		GpuMat* pSrc;
		GpuMat* pDest;
		GpuMat* pTmp;


		//BGR
		m_pZed->retrieveImage(*m_pzImg, (sl::VIEW)m_zedViewLR, sl::MEM_GPU);
#ifndef USE_OPENCV4TEGRA
		cuda::cvtColor(m_gImg, m_gImg2, CV_BGRA2BGR);
#else
		gpu::cvtColor(m_gImg, m_gImg2, CV_BGRA2BGR);
#endif
		pSrc = &m_gImg2;
		pDest = &m_gImg;

		if (m_bFlip)
		{
#ifndef USE_OPENCV4TEGRA
			cuda::flip(*pSrc, *pDest, -1);
#else
			gpu::flip(*pSrc,*pDest,-1);
#endif

			SWAP(pSrc, pDest, pTmp);
		}
		m_pBGR->update(pSrc);
		if(m_pGray)
			m_pGray->getGrayOf(m_pBGR);
		if (m_pHSV)
			m_pHSV->getHSVOf(m_pBGR);


		//Depth
		m_pZed->retrieveMeasure(*m_pzDepth, sl::MEASURE_DEPTH, sl::MEM_GPU);
		pSrc = &m_gDepth;
		pDest = &m_gDepth2;

		if (m_bFlip)
		{
#ifndef USE_OPENCV4TEGRA
			cuda::flip(*pSrc, *pDest, -1);
#else
			gpu::flip(*pSrc,*pDest,-1);
#endif

			SWAP(pSrc, pDest, pTmp);
		}
		m_pDepth->update(pSrc);


		//Normalized Depth
		if(m_pDepthNorm)
		{
			if (m_depthNormInt.x != m_depthNormInt.y)
			{
				double alpha = 255.0/(m_depthNormInt.y-m_depthNormInt.x);
				double beta = -alpha * m_depthNormInt.x;

				m_pDepth->getGMat()->convertTo(*m_pDepthNorm->getGMat(), CV_8U, alpha, beta);
				m_pDepthNorm->updatedGMat();
			}
		}


		//Tracking
		m_zedTrackState = m_pZed->getPosition(m_zedCamPose, sl::REFERENCE_FRAME_LAST);

		if (m_zedTrackState == sl::TRACKING_STATE_OK)
		{
			sl::Vector3<float> rot = m_zedCamPose.getRotationVector();
			m_vR.x += (double)rot.x;
			m_vR.y += (double)rot.y;
			m_vR.z += (double)rot.z;

			sl::Vector3<float> trans = m_zedCamPose.getTranslation();
			m_vT.x += (double)trans.tx;
			m_vT.y += (double)trans.ty;
			m_vT.z += (double)trans.tz;

			m_trackConfidence += m_zedCamPose.pose_confidence;
			m_trackCount++;

			uint64_t trackT = m_pZed->getCameraTimestamp();
			m_trackDT += trackT - m_tLastTrack;
			m_tLastTrack = trackT;
		}
		else
		{
			resetMotionDelta();
		}

		this->autoFPSto();
	}
}

int _ZED::getMotionDelta(vDouble3* pT, vDouble3* pR, uint64_t* pDT)
{
	if(m_zedTrackState != sl::TRACKING_STATE_OK)
		return -1;

	*pT = m_vT;
	*pR = m_vR;
	*pDT = m_trackDT / 1000;

	int avrConfidence = m_trackConfidence;
	int tCount = m_trackCount;
	if(tCount > 0)avrConfidence /= tCount;

	resetMotionDelta();

	return avrConfidence;
}

void _ZED::resetMotionDelta(void)
{
	m_vT.init();
	m_vR.init();
	m_trackDT = 0;
	m_trackConfidence = 0;
	m_trackCount = 0;
}

void _ZED::setAttitude(vDouble3* pYPR)
{
	NULL_(pYPR);
//	IF_(m_trackState != tracking);

//	Eigen::AngleAxisf y(pYPR->x, Eigen::Vector3f::UnitY());
//	Eigen::AngleAxisf p(pYPR->y, Eigen::Vector3f::UnitX());
//	Eigen::AngleAxisf r(pYPR->z, Eigen::Vector3f::UnitZ());
//
//	Eigen::Quaternion<float> q = y * p * r;
//	Eigen::Matrix3f mRot = q.matrix();

//	m_zed->setTrackingPrior(mRot);
}

vDouble2 _ZED::range(void)
{
	vDouble2 r;
	r.x = m_zedMinDist;
	r.y = m_zedMaxDist;
	return r;
}

double _ZED::dist(Rect* pR)
{
	NULL_F(pR);

	GpuMat gMat;
	GpuMat gMat2;
	GpuMat gHist;
	Mat cHist;

	NULL_F(m_pDepth);
	IF_F(m_pDepth->empty());
	gMat = *(m_pDepth->getGMat());
	gMat2 = GpuMat(gMat, *pR);

	int intensity = 0;
	int minPix = pR->area() * 0.5;

#ifndef USE_OPENCV4TEGRA
	cuda::calcHist(gMat2, gHist);
	gHist.download(cHist);

	for (int i = cHist.cols - 1; i > 0; i--)
	{
		intensity += cHist.at<int>(0, i);
		if (intensity > minPix)
		{
			return (255.0f - i) / 255.0f;
		}
	}
#else
	int channels[] =
	{	0};
	int bin_num = 256;
	int bin_nums[] =
	{	bin_num};
	float range[] =
	{	0, 256};
	const float *ranges[] =
	{	range};
	Mat cMat;
	gMat2.download(cMat);
	cv::calcHist(&cMat, 1, channels, cv::Mat(), cHist, 1, bin_nums, ranges);

	for (int i = cHist.rows-1; i > 0; i--)
	{
		intensity += cHist.at<int>(i, 0);
		if(intensity > minPix)
		{
			return (255.0f - i)/255.0f;
		}
	}
#endif

	return -1.0;
}

bool _ZED::draw(void)
{
	Window* pWin;
	Frame* pFrame;

	if (this->BASE::draw())
	{
		IF_F(m_pBGR->empty());
		pWin = (Window*) this->m_pWindow;
		pFrame = pWin->getFrame();
		pFrame->update(m_pBGR);
		this->_VisionBase::draw();

		string msg;
		pWin->tabNext();

		int avrConfidence = m_trackConfidence;
		int tCount = m_trackCount;
		if(tCount > 0)avrConfidence /= tCount;

		msg = "Tracking confidence: " + i2str(avrConfidence);
		pWin->addMsg(&msg);

		msg = "Translation: X=" + f2str(m_vT.x) + ", Y=" + f2str(m_vT.y)
				+ ", Z=" + f2str(m_vT.z);
		pWin->addMsg(&msg);

		msg = "Rotation: Yaw=" + f2str(m_vR.x) + ", Pitch=" + f2str(m_vR.y)
				+ ", Roll=" + f2str(m_vR.z);
		pWin->addMsg(&msg);

		pWin->tabPrev();
	}

	if (m_pDepthWin && m_pDepthNorm)
	{
		pFrame = m_pDepthWin->getFrame();
		if (pFrame && !m_pDepthNorm->empty())
		{
			pFrame->update(m_pDepthNorm);
		}
	}

	return true;
}

}

#endif

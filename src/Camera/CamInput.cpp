/*
 * CamInput.cpp
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#include "CamInput.h"

namespace kai
{

CamInput::CamInput()
{
	m_camType = CAM_GENERAL;
	m_width = 0;
	m_height = 0;
	m_centerH = 0;
	m_centerV = 0;
	m_camDeviceID = 0;
	m_bCalibration = false;
	m_bGimbal = false;
	m_bFisheye = false;
	m_isoScale = 1.0;
	m_rotTime = 0;
	m_rotPrev = 0;
	m_angleH = 0;
	m_angleV = 0;
}

CamInput::~CamInput()
{
}

bool CamInput::setup(JSON* pJson, string camName)
{
	string presetDir = "";
	string calibFile;

	CHECK_INFO(pJson->getVal("PRESET_DIR", &presetDir));

	CHECK_FATAL(pJson->getVal("CAM_" + camName + "_ID", &m_camDeviceID));
	CHECK_FATAL(pJson->getVal("CAM_" + camName + "_WIDTH", &m_width));
	CHECK_FATAL(pJson->getVal("CAM_" + camName + "_HEIGHT", &m_height));
	CHECK_FATAL(pJson->getVal("CAM_" + camName + "_ANGLE_V", &m_angleV));
	CHECK_FATAL(pJson->getVal("CAM_" + camName + "_ANGLE_H", &m_angleH));

	CHECK_INFO(pJson->getVal("CAM_" + camName + "_ISOSCALE", &m_isoScale));
	CHECK_INFO(pJson->getVal("CAM_" + camName + "_CALIB", &m_bCalibration));
	CHECK_INFO(pJson->getVal("CAM_" + camName + "_GIMBAL", &m_bGimbal));
	CHECK_INFO(pJson->getVal("CAM_" + camName + "_FISHEYE", &m_bFisheye));
	CHECK_INFO(pJson->getVal("CAM_" + camName + "_TYPE", &m_camType));

	if (pJson->getVal("CAM_" + camName + "_CALIBFILE", &calibFile))
	{
		FileStorage fs(presetDir + calibFile, FileStorage::READ);
		if (!fs.isOpened())
		{
			LOG(ERROR)<<"Camera calibration file not found";
			m_bCalibration = false;
		}
		else
		{
			fs["camera_matrix"] >> m_cameraMat;
			fs["distortion_coefficients"] >> m_distCoeffs;
			fs.release();

			Mat map1, map2;
			cv::Size imSize(m_width,m_height);

			if(m_bFisheye)
			{
				Mat newCamMat;
				fisheye::estimateNewCameraMatrixForUndistortRectify(m_cameraMat, m_distCoeffs, imSize,
						Matx33d::eye(), newCamMat, 1);
				fisheye::initUndistortRectifyMap(m_cameraMat, m_distCoeffs, Matx33d::eye(), newCamMat, imSize,
						CV_32F/*CV_16SC2*/, map1, map2);
			}
			else
			{
				initUndistortRectifyMap(
						m_cameraMat,
						m_distCoeffs,
						Mat(),
						getOptimalNewCameraMatrix(
								m_cameraMat,
								m_distCoeffs,
								imSize,
								1,
								imSize,
								0),
						imSize,
						CV_32FC1,
						map1,
						map2);
			}

			m_Gmap1.upload(map1);
			m_Gmap2.upload(map2);

			LOG(INFO)<<"Camera calibration initialized";

		}
	}

	return true;
}

bool CamInput::openCamera(void)
{
#ifdef USE_ZED
	if (m_camType == CAM_ZED)
	{
		// Initialize ZED color stream in HD and depth in Performance mode
		m_pZed = new sl::zed::Camera(sl::zed::HD1080);

		// define a struct of parameters for the initialization
		sl::zed::InitParams zedParams;
		zedParams.mode = sl::zed::MODE::PERFORMANCE;
		zedParams.unit = sl::zed::UNIT::MILLIMETER;
		zedParams.verbose = 1;
		zedParams.device = -1;
		zedParams.minimumDistance = 1000;

		sl::zed::ERRCODE err = m_pZed->init(zedParams);
		if (err != sl::zed::SUCCESS)
		{
			LOG(ERROR)<< "ZED Error code: " << sl::zed::errcode2str(err) << std::endl;
			return false;
		}

		// Initialize color image and depth
		m_width = m_pZed->getImageSize().width;
		m_height = m_pZed->getImageSize().height;

		m_centerH = m_width * 0.5;
		m_centerV = m_height * 0.5;

		m_zedMode = sl::zed::STANDARD; //FULL

		m_frame = Mat(m_height, m_width, CV_8UC4, 1);
		m_depthFrame = Mat(m_height, m_width, CV_8UC4, 1);

		return true;
	}
#endif

	m_camera.open(m_camDeviceID);
	if (!m_camera.isOpened())
	{
		LOG(ERROR)<< "Cannot open camera:" << m_camDeviceID;
		return false;
	}

	m_camera.set(CV_CAP_PROP_FRAME_WIDTH, m_width);
	m_camera.set(CV_CAP_PROP_FRAME_HEIGHT, m_height);

	m_centerH = m_width * 0.5;
	m_centerV = m_height * 0.5;

	return true;
}

GpuMat* CamInput::readFrame(void)
{
	GpuMat* pSrc;
	GpuMat* pDest;
	GpuMat* pTmp;

#ifdef USE_ZED
	if (m_camType == CAM_ZED)
	{
		// Grab frame and compute depth in FULL sensing mode
		if (!m_pZed->grab(m_zedMode))
		{
			// Retrieve left color image
			sl::zed::Mat gLeft = m_pZed->retrieveImage_gpu(sl::zed::SIDE::LEFT);
			m_Gframe = GpuMat(Size(m_width,m_height), CV_8UC4, gLeft.data);

			// Retrieve depth map
			sl::zed::Mat gDepth = m_pZed->normalizeMeasure_gpu(sl::zed::MEASURE::DEPTH);
			m_Gdepth = GpuMat(Size(m_width,m_height), CV_8UC4, gDepth.data);

			cuda::cvtColor(m_Gframe, m_Gframe2, CV_BGRA2BGR);
			cuda::cvtColor(m_Gdepth, m_Gdepth2, CV_BGRA2GRAY);

			return &m_Gframe2;
		}

		return NULL;
	}
#endif

	while (!m_camera.read(m_frame));
	m_Gframe.upload(m_frame);

	pSrc = &m_Gframe;
	pDest = &m_Gframe2;

	if (m_bCalibration)
	{
		cuda::remap(*pSrc, *pDest, m_Gmap1, m_Gmap2, INTER_LINEAR);
		SWAP(pSrc, pDest, pTmp);
	}

	if (m_bGimbal)
	{
		cuda::warpAffine(*pSrc, *pDest, m_rotRoll, m_Gframe.size());
		SWAP(pSrc, pDest, pTmp);
	}

	return pSrc;
}

GpuMat* CamInput::getDepthFrame(void)
{
	if (m_camType == CAM_ZED)
	{
		return &m_Gdepth2;
	}

	return NULL;
}

void CamInput::setAttitude(double rollRad, double pitchRad, uint64_t timestamp)
{
	Point2f center(m_centerH, m_centerV);
	double deg = -rollRad * 180.0 * OneOvPI;

	m_rotRoll = getRotationMatrix2D(center, deg, m_isoScale);

	//TODO: add rot estimation

}

int CamInput::getType(void)
{
	return m_camType;
}

} /* namespace kai */

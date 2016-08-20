/*
 * CamInput.h
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#ifndef SRC_CAMINPUT_H_
#define SRC_CAMINPUT_H_

#include "stdio.h"
#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "../Camera/Frame.h"

#define CAM_GENERAL 0
#define CAM_ZED 1

namespace kai
{

class Camera {
public:
	Camera();
	virtual ~Camera();

	bool setup(JSON* pJson, string camName);

	void setCameraID(int);
	bool openCamera(void);

	GpuMat* readFrame(void);
	GpuMat* getDepthFrame(void);

	void setAttitude(double rollRad, double pitchRad, uint64_t timestamp);
	int	getType(void);

public:

#ifdef USE_ZED
	sl::zed::Camera* m_pZed;
	sl::zed::SENSING_MODE m_zedMode;
	int	m_zedResolution;
	double m_zedMinDist;
#endif

	int m_camType;
	int m_camDeviceID;
	VideoCapture m_camera;

	double m_width;
	double m_height;
	double m_centerH;
	double m_centerV;
	double m_angleH;
	double m_angleV;

	Mat m_depthFrame;
	Mat m_frame;
	Mat m_cameraMat;
	Mat m_distCoeffs;
	GpuMat m_Gmap1;
	GpuMat m_Gmap2;
	bool m_bCalibration;
	bool m_bFisheye;

	bool m_bGimbal;
	Mat m_rotRoll;
	uint64_t m_rotTime;
	double m_rotPrev;
	double m_isoScale;

	int		m_bCrop;
	Rect	m_cropBB;

//private:
public:
	GpuMat m_Gframe;
	GpuMat m_Gframe2;
	GpuMat m_Gdepth;
	GpuMat m_Gdepth2;

};

} /* namespace kai */

#endif /* SRC_CAMINPUT_H_ */

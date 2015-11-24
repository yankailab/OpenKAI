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
#include "../Camera/CamFrame.h"

namespace kai
{

class CamInput
{
public:
	CamInput();
	virtual ~CamInput();

	bool setup(JSON* pJson, string camName);

	void setCameraID(int);
	bool openCamera(void);

	void readFrame(CamFrame* pFrame);
	void setAttitude(double rollRad, double pitchRad, uint64_t timestamp);

public:
	int 			m_camDeviceID;
	VideoCapture m_camera;

	int 			m_width;
	int 			m_height;
	int			m_centerH;
	int			m_centerV;

	Mat 			m_frame;
	Mat 			m_cameraMat;
	Mat 			m_distCoeffs;
	GpuMat		m_Gmap1;
	GpuMat		m_Gmap2;
	bool			m_bCalibration;

	bool			m_bGimbal;
	Mat			m_rotRoll;
	uint64_t		m_rotTime;
	double		m_rotPrev;
	double		m_isoScale;

private:
	GpuMat 		m_Gframe;
	GpuMat 		m_Gframe2;

};

} /* namespace kai */

#endif /* SRC_CAMINPUT_H_ */

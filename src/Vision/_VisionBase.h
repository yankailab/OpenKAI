/*
 * VisionBase.h
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__VisionBase_H_
#define OpenKAI_src_Vision__VisionBase_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"

namespace kai
{

enum VISION_TYPE
{
	unknownVision,
	camera,
	gstreamer,
	video,
	zed,
	realsense,
	pylon,
	raspivid,
};

class _VisionBase: public _ThreadBase
{
public:
	_VisionBase();
	virtual ~_VisionBase();

	virtual bool init(void* pKiss);
	virtual bool draw(void);

	virtual VISION_TYPE getType(void);
	virtual Frame* BGR(void);
	virtual Frame* HSV(void);
	virtual Frame* Gray(void);
	virtual Mat* K(void);
	virtual void info(vInt2* pSize, vInt2* pCenter, vDouble2* pAngle);

	void postProcess(void);
	void setAttitude(double rollRad, double pitchRad, uint64_t timestamp);
	bool isOpened(void);

public:
	bool m_bOpen;
	VISION_TYPE m_type;

	int m_w;
	int m_h;
	int m_cW;
	int m_cH;
	double m_fovW;
	double m_fovH;
	bool m_bFlip;

	//calibration
	bool m_bCalibration;
	bool m_bFisheye;

	//crop
	bool m_bCrop;
	Rect m_cropBB;

	//gimbal
	Mat m_K;
	bool m_bGimbal;
	Mat m_rotRoll;
	uint64_t m_rotTime;
	double m_rotPrev;
	double m_isoScale;

	//frame
	Frame	m_fBGR;
	Frame*	m_pGray;
	Frame*	m_pHSV;

	//post processing thread
	_ThreadBase* m_pTPP;

};

}

#endif

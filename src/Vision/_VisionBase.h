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
	video,
	zed,
	realsense,
	pylon,
};

class _VisionBase: public _ThreadBase
{
public:
	_VisionBase();
	virtual ~_VisionBase();

	virtual bool init(void* pKiss);
	virtual void reset(void);
	virtual bool draw(void);

	virtual VISION_TYPE getType(void);
	virtual Frame* BGR(void);
	virtual Frame* HSV(void);
	virtual Frame* Gray(void);

	virtual Mat* K(void);
	virtual void info(vInt2* pSize, vInt2* pCenter, vInt2* pAngle);

	void setAttitude(double rollRad, double pitchRad, uint64_t timestamp);
	bool isOpened(void);

public:
	bool m_bOpen;
	VISION_TYPE m_type;

	int m_w;
	int m_h;
	int m_cW;
	int m_cH;
	int m_fovW;
	int m_fovH;
	bool m_bFlip;

	Mat m_K;
	bool m_bGimbal;
	Mat m_rotRoll;
	uint64_t m_rotTime;
	double m_rotPrev;
	double m_isoScale;

	//frame
	Mat  	m_mTemp;
	GpuMat  m_mgTemp;
	Frame*	m_pBGR;
	Frame*	m_pGray;
	Frame*	m_pHSV;
};

}

#endif

/*
 * StreamBase.h
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#ifndef SRC_STREAM_STREAMBASE_H_
#define SRC_STREAM_STREAMBASE_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"

namespace kai
{

enum STREAM_TYPE
{
	unknownStream,
	camera,
	video,
	zed,
};

class _StreamBase: public _ThreadBase
{
public:
	_StreamBase();
	virtual ~_StreamBase();

	virtual bool init(void* pKiss);
	virtual STREAM_TYPE getType(void);

	virtual Frame* bgr(void);
	virtual Frame* hsv(void);
	virtual Frame* gray(void);
	virtual Frame* depth(void);

	virtual bool distNearest(vDouble4* pRect, double* pDist, double* pSize);

	void setAttitude(double rollRad, double pitchRad, uint64_t timestamp);

public:
	bool m_bOpen;
	STREAM_TYPE m_type;

	int m_width;
	int m_height;
	int m_centerH;
	int m_centerV;
	int m_angleH;
	int m_angleV;

	Mat m_mat;

	bool m_bGimbal;
	Mat m_rotRoll;
	uint64_t m_rotTime;
	double m_rotPrev;
	double m_isoScale;

	//frame
	Frame*	m_pBGR;
	Frame*	m_pGray;
	Frame*	m_pHSV;
	Frame*	m_pDepth;

	//operation
	GpuMat m_Gframe;
	GpuMat m_Gframe2;


};

}

#endif

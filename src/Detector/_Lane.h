/*
 * _Lane.h
 *
 *  Created on: Mar 5, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__Lane_H_
#define OpenKAI_src_Detector__Lane_H_

#include "../Base/common.h"
#include "../Vision/_VisionBase.h"
#include "../Filter/Median.h"
#include "../Filter/Average.h"

#define N_LANE_BLOCK 100

namespace kai
{

struct LANE_BLOCK
{
	vDouble4 m_fROI;
	Rect m_iROI;
	int m_v;

	void init(void)
	{
		m_fROI.init();
	}

	void f2iROI(vInt2 camSize)
	{
		m_iROI.x = ((double)camSize.x * m_fROI.x);
		m_iROI.y = ((double)camSize.y * m_fROI.y);
		m_iROI.width = ((double)camSize.x * m_fROI.z) - m_iROI.x;
		m_iROI.height = ((double)camSize.y * m_fROI.w) - m_iROI.y;
	}
};

struct LANE_DETECTION
{
	Median m_fMed;
	Average m_fAvr;
	double m_v;

	void init(void* pKmed, void* pKavr)
	{
		m_fMed.init(pKmed);
		m_fAvr.init(pKavr);
		m_v = 0.0;
	}

	void input(double v)
	{
		m_v = v;
		m_fMed.input(v);
		m_fAvr.input(m_fMed.v());
	}

	double v(void)
	{
		return m_v;
	}
};

struct LANE
{
	double m_QoD;	//Quality of Detection
	LANE_DETECTION m_laneDet[N_LANE_BLOCK];
};

class _Lane: public _ThreadBase
{
public:
	_Lane();
	virtual ~_Lane();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);

private:
	void updateVisionSize(void);
	void getOverheadBin(void);
	void updateBlock(void);
	void updateLaneBlock(void);
	void updateLane(void);
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Lane*) This)->update();
		return NULL;
	}

	_VisionBase* m_pVision;
	vInt2 m_vSize;

	vDouble2 m_roiLT;
	vDouble2 m_roiLB;
	vDouble2 m_roiRT;
	vDouble2 m_roiRB;
	Mat m_mPerspective;
	vInt2 m_sizeOverhead;
	Mat m_mOverhead;
	Mat m_mBin;

	Mat m_mLAB;
	Mat m_mHSV;
	Mat m_mHLS;
	cv::Ptr<cv::CLAHE> m_pClaheLAB;
	cv::Ptr<cv::CLAHE> m_pClaheHSV;
	cv::Ptr<cv::CLAHE> m_pClaheHLS;
	int m_tileClahe;
	double m_clipLimLAB;
	double m_clipLimHSV;
	double m_clipLimHLS;
	int m_thrLAB;
	int m_thrHSV;
	int m_thrHLS;

	double m_w;
	double m_h;
	double m_dW;
	double m_dH;
	int m_nBlockX;
	int m_nBlockY;
	int m_nBlock;
	LANE_BLOCK* m_pBlock;

	LANE m_laneL;
	LANE m_laneR;

	bool m_bDrawOverhead;
	bool m_bDrawBin;



};

}
#endif

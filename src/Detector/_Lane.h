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

namespace kai
{

struct LANE
{
	vDouble4 m_roi;
	vector<Vec4i> m_vLine;
	vector<Point> m_vPoint;

};

struct LANE_BLOCK
{
	vDouble4 m_fROI;
	vInt4 m_iROI;

	void init(void)
	{
		m_fROI.init();
		m_iROI.init();
	}
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

	Mat m_mLAB;
	Mat m_mHSV;
	Mat m_mHLS;

	double m_w;
	double m_h;
	double m_dW;
	double m_dH;
	int m_nBlockX;
	int m_nBlockY;
	int m_nBlock;
	LANE_BLOCK* m_pBlock;



};

}
#endif

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

#define N_LANE_FILTER 3

namespace kai
{

struct LANE_FILTER
{
	Mat m_mBin;
	int m_iColorSpace;
	int m_iChannel;
	int m_nTile;
	int m_thr;
	double m_clipLim;
	cv::Ptr<cv::CLAHE> m_pClahe;

	void init(void)
	{
		m_pClahe = cv::createCLAHE(m_clipLim, Size(m_nTile, m_nTile));
	}

	Mat filter(Mat mIn)
	{
		vector<Mat> vMat(3);
		Mat mColor;
		Mat mClahe;

		cv::cvtColor(mIn, mColor, m_iColorSpace);
		split(mColor, vMat);
		m_pClahe->apply(vMat[m_iChannel], mClahe);
		cv::threshold(mClahe, m_mBin, m_thr, 1, cv::THRESH_BINARY);

		return m_mBin;
	}
};

struct LANE
{
	Median* m_pMed;
	Average* m_pAvr;
	vector<double> m_vX;
	vector<double> m_vY;
	vector<double> m_vPolyFit;

	void init(int n, int nAvr, int nMed)
	{
		m_pMed = new Median[n];
		m_pAvr = new Average[n];

		for(int i=0; i<n; i++)
		{
			m_pMed[i].init(nMed,0);
			m_pAvr[i].init(nAvr,0);
		}

		m_vPolyFit.clear();
	}

	void input(int i, double v)
	{
		m_pMed[i].input(v);
		m_pAvr[i].input(m_pMed[i].v());
	}

	double v(int i)
	{
		return m_pAvr[i].v();
	}

	void polyFit(void)
	{
		m_vX.clear();
		m_vY.clear();

		int n = 0;


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
	void filterBin(void);
	void findLane(void);
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

	int m_nFilter;
	LANE_FILTER m_pFilter[N_LANE_FILTER];

	LANE m_laneL;
	LANE m_laneR;

	bool m_bDrawOverhead;
	bool m_bDrawFilter;



};

}
#endif

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
#include <gsl/gsl_multifit.h>

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
	int m_n;
	Median* m_pMed = NULL;
	Average* m_pAvr = NULL;

	double* m_pX = NULL;
	double* m_pY = NULL;
	double* m_pPoly = NULL;
	int m_degPoly;
	gsl_multifit_linear_workspace *m_gWS;
	gsl_matrix *m_gCov, *m_gX;
	gsl_vector *m_gY, *m_gC;

	void init(int n, int nAvr, int nMed)
	{
		m_n = n;
		m_pMed = new Median[n];
		m_pAvr = new Average[n];

		for (int i = 0; i < n; i++)
		{
			m_pMed[i].init(nMed, 0);
			m_pAvr[i].init(nAvr, 0);
		}

		m_pX = new double[n];
		m_pY = new double[n];
		m_degPoly = 3;
		m_pPoly = new double[m_degPoly];

		m_gX = gsl_matrix_alloc(n, m_degPoly);
		m_gY = gsl_vector_alloc(n);
		m_gC = gsl_vector_alloc(m_degPoly);
		m_gCov = gsl_matrix_alloc(m_degPoly, m_degPoly);
	}

	void reset(void)
	{
		DEL(m_pMed);
		DEL(m_pAvr);
		DEL(m_pX);
		DEL(m_pY);
		DEL(m_pPoly);

		gsl_multifit_linear_free(m_gWS);
		gsl_matrix_free(m_gX);
		gsl_matrix_free(m_gCov);
		gsl_vector_free(m_gY);
		gsl_vector_free(m_gC);
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

	void poly(void)
	{
		double chisq;
		int i, j;

		for (i = 0; i < m_n; i++)
		{
			for (j = 0; j < m_degPoly; j++)
			{
				gsl_matrix_set(m_gX, i, j, pow(i, j));
			}
			gsl_vector_set(m_gY, i, m_pAvr[i].v());
		}

		m_gWS = gsl_multifit_linear_alloc(m_n, m_degPoly);
		gsl_multifit_linear(m_gX, m_gY, m_gC, m_gCov, &chisq, m_gWS);

		for (i = 0; i < m_degPoly; i++)
		{
			m_pPoly[i] = gsl_vector_get(m_gC, i);
		}
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
	Mat m_mPerspectiveInv;
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

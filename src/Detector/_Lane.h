/*
 * _Lane.h
 *
 *  Created on: Mar 5, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__Lane_H_
#define OpenKAI_src_Detector__Lane_H_

#include "../Vision/_VisionBase.h"

#ifdef USE_OPENCV
#include "../Filter/Median.h"
#include "../Filter/Average.h"
#include <gsl/gsl_multifit.h>

#define N_LANE_FILTER 3
#define N_LANE 3

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
	Median<float>* m_pMed = NULL;
	Average<float>* m_pAvr = NULL;
	double m_deviation;
	vDouble4 m_ROI;

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
		m_deviation = 0.0;
		m_ROI.init();
		m_ROI.z = 1.0;
		m_ROI.w = 1.0;
		m_pMed = new Median<float>[n];
		m_pAvr = new Average<float>[n];

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

	double vFilter(int i)
	{
		return m_pAvr[i].v();
	}

	void findLane(Mat& mBin)
	{
		double vMax;
		Point iMax;

		Rect r;
		r.x = m_ROI.x * mBin.cols;
		r.y = m_ROI.y * mBin.rows;
		r.width = m_ROI.z * mBin.cols - r.x;
		r.height = m_ROI.w * mBin.rows - r.y;
		Mat mROI = mBin(r);

		Rect rX;
		rX.x = 0;
		rX.width = r.width;
		rX.height = 1;

		for(int i=0; i<mROI.rows; i++)
		{
			rX.y = i;
			cv::minMaxLoc(mBin(rX),NULL,&vMax,NULL,&iMax);
			IF_CONT(vMax <= 0);

			input(r.y+i, r.x+iMax.x);
		}
	}

	void updatePolyFit(void)
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

	double vPoly(int iY)
	{
		double iX = 0;
		double pow = 1.0;

		for (int i = 0; i < m_degPoly; i++)
		{
			iX += m_pPoly[i] * pow;
			pow *= (double) iY;
		}

		return iX;
	}

	void updateDeviation(void)
	{
		double d = 0.0;
		for (int i = 0; i < m_n; i++)
		{
			d += abs(vFilter(i)-vPoly(i));
		}

		m_deviation = d/(double)m_n;
	}

};

class _Lane: public _ModuleBase
{
public:
	_Lane();
	virtual ~_Lane();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);
	int check(void);

private:
	void updateVisionSize(void);
	void filterBin(void);
	void detect(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		((_Lane*) This)->update();
		return NULL;
	}

	_VisionBase* m_pV;
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
	int m_binMed;

	int m_nFilter;
	LANE_FILTER m_pFilter[N_LANE_FILTER];

	int m_nLane;
	LANE m_pLane[N_LANE];

	Point** m_ppPoint;
	int* m_pNp;
	bool m_bDrawOverhead;
	bool m_bDrawFilter;

};

}
#endif
#endif

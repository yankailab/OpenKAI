/*
 * PolyFit.cpp
 *
 *  Created on: Sept 12, 2018
 *      Author: yankai
 */

#include "PolyFit.h"

namespace kai
{

	PolyFit::PolyFit()
	{
		m_pY = NULL;
		m_pPoly = NULL;
		m_n = 0;
		m_nDeg = 2;

		m_gWS = NULL;
		m_gCov = NULL;
		m_gX = NULL;
		m_gY = NULL;
		m_gC = NULL;
	}

	PolyFit::~PolyFit()
	{
		DEL(m_pY);
		DEL(m_pPoly);

		gsl_multifit_linear_free(m_gWS);
		gsl_matrix_free(m_gX);
		gsl_matrix_free(m_gCov);
		gsl_vector_free(m_gY);
		gsl_vector_free(m_gC);
	}

	bool PolyFit::init(int n, int nDeg)
	{
		m_n = n;
		m_pY = new double[m_n];

		m_nDeg = nDeg;
		m_pPoly = new double[m_nDeg];

		m_gX = gsl_matrix_alloc(m_n, m_nDeg);
		m_gY = gsl_vector_alloc(m_n);
		m_gC = gsl_vector_alloc(m_nDeg);
		m_gCov = gsl_matrix_alloc(m_nDeg, m_nDeg);
		m_gWS = gsl_multifit_linear_alloc(m_n, m_nDeg);

		return true;
	}

	void PolyFit::x(int i, double y)
	{
		IF_(i >= m_n);

		m_pY[i] = y;
	}

	void PolyFit::fit(void)
	{
		double chisq;
		int i, j;

		for (i = 0; i < m_n; i++)
		{
			for (j = 0; j < m_nDeg; j++)
			{
				gsl_matrix_set(m_gX, i, j, pow(i, j));
			}
			gsl_vector_set(m_gY, i, m_pY[i]);
		}

		gsl_multifit_linear(m_gX, m_gY, m_gC, m_gCov, &chisq, m_gWS);

		for (i = 0; i < m_nDeg; i++)
		{
			m_pPoly[i] = gsl_vector_get(m_gC, i);
		}
	}

	double PolyFit::yPoly(int x)
	{
		double y = 0;
		double pow = 1.0;

		for (int i = 0; i < m_nDeg; i++)
		{
			y += m_pPoly[i] * pow;
			pow *= (double)x;
		}

		return y;
	}

}

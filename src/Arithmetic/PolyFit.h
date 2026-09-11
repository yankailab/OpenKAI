/*
 * PolyFit.h
 *
 *  Created on: Sept 12, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Arithmetic_PolyFit_H_
#define OpenKAI_src_Arithmetic_PolyFit_H_

#include "../Base/common.h"
#include <gsl/gsl_multifit.h>

namespace kai
{

	class PolyFit
	{
	public:
		PolyFit();
		virtual ~PolyFit();

		virtual bool init(int n, int nDeg);
		virtual void x(int i, double y);
		virtual void fit(void);
		virtual double yPoly(int x);

	protected:
		gsl_multifit_linear_workspace *m_gWS = nullptr;
		gsl_matrix *m_gCov = nullptr, *m_gX = nullptr;
		gsl_vector *m_gY = nullptr, *m_gC = nullptr;

		int m_n = 0;
		int m_nDeg = 2;
		double *m_pY = nullptr;
		double *m_pPoly = nullptr;
	};

}
#endif

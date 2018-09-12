/*
 * PolyFit.h
 *
 *  Created on: Sept 12, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Filter_PolyFit_H_
#define OpenKAI_src_Filter_PolyFit_H_

#include "../Base/common.h"
#include <gsl/gsl_multifit.h>

namespace kai
{

class PolyFit
{
public:
	PolyFit();
	virtual ~PolyFit();

	bool init(int n, int nDeg);
	void x(int i, double y);
	void fit(void);
	double yPoly(int x);

public:
	gsl_multifit_linear_workspace *m_gWS;
	gsl_matrix *m_gCov, *m_gX;
	gsl_vector *m_gY, *m_gC;

	int m_n;
	int m_nDeg;
	double* m_pY;
	double* m_pPoly;

};

}
#endif

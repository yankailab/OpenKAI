/*
 * Destimator.cpp
 *
 *  Created on: Mar 22, 2021
 *      Author: yankai
 */

#include "Destimator.h"

namespace kai
{

	Destimator::Destimator()
	{
	}

	Destimator::~Destimator()
	{
	}

	bool Destimator::init(void *pKiss)
	{
		IF_F(!this->BASE::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;
		pK->m_pInst = this;

		//	pK->v("vMin",&m_vMin);

		return true;
	}

	double Destimator::v(int x)
	{
		return 0.0; // m_vOut;
	}

}

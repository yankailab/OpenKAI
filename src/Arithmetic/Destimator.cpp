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

	int Destimator::init(void *pKiss)
	{
		CHECK_(this->BASE::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		return OK_OK;
	}

	double Destimator::v(int x)
	{
		return 0.0; // m_vOut;
	}

}

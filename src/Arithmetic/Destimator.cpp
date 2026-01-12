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

	int Destimator::init(const json& j)
	{
		CHECK_(this->BASE::init(j));

		return true;
	}

	double Destimator::v(int x)
	{
		return 0.0; // m_vOut;
	}

}

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

	bool Destimator::init(const json &j)
	{
		IF_F(!this->BASE::init(j));

		return true;
	}

	double Destimator::v(int x)
	{
		return 0.0; // m_vOut;
	}

}

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

	bool Destimator::loadConfig(void)
	{
		IF_F(!this->BASE::loadConfig());

		return true;
	}

	double Destimator::v(int x)
	{
		return 0.0; // m_vOut;
	}

}

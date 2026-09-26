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

	bool Destimator::saveConfig(bool bExport)
	{
		if (!BASE::saveConfig(false))
		{
			return false;
		}

		if (!bExport)
		{
			return true;
		}
		return m_pJcfg->saveToFile();
	}

	double Destimator::v(int x)
	{
		return 0.0; // m_vOut;
	}

}

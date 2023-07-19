/*
 * GeoGrid.cpp
 *
 *  Created on: July 19, 2023
 *      Author: yankai
 */

#include "GeoGrid.h"

namespace kai
{

	GeoGrid::GeoGrid()
	{
	}

	GeoGrid::~GeoGrid()
	{
	}

	bool GeoGrid::init(void *pKiss)
	{
		IF_F(!this->BASE::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		return true;
	}

	bool GeoGrid::link(void)
	{
		IF_F(!this->BASE::link());

		return true;
	}

	int GeoGrid::check(void)
	{
		return BASE::check();
	}

	int GeoGrid::gen(const GEOGRID_AXIS &lat, const GEOGRID_AXIS &lng, const GEOGRID_AXIS &alt)
	{
	}

	int GeoGrid::getCellIdx(const vDouble3& p)
	{
	}

	GEOGRID_CELL GeoGrid::getCell(uint32_t cIdx)
	{
	}

	void GeoGrid::getCoverage(const vDouble4& pCover, int* pCidx, float* pCoverage)
	{

	}

	void GeoGrid::console(void *pConsole)
	{
		this->BASE::console(pConsole);
	}

}

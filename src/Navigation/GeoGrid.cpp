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
		m_nCell = 0;
		m_nLL = 0;
		m_gCode = 0;
	}

	GeoGrid::~GeoGrid()
	{
	}

	bool GeoGrid::init(void *pKiss)
	{
		IF_F(!this->BASE::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;
		pK->m_pInst = this;

		pK->m_pInst = this;

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
		m_alt = alt;
		m_lat = lat;
		m_lng = lng;

		IF__(!m_alt.gen(), -1);
		IF__(!m_lat.gen(), -1);
		IF__(!m_lng.gen(), -1);

		m_nLL = m_lat.m_nDiv * m_lng.m_nDiv;
		m_nCell = m_alt.m_nDiv * m_nLL;
		return m_nCell;
	}

	int32_t GeoGrid::getCellIdx(const vDouble3 &p)
	{
		IF__(m_nCell < 0, -1);

		int iAlt = m_alt.getIdx(p.z);
		IF__(iAlt < 0, -1);

		int iLat = m_lat.getIdx(p.x);
		IF__(iLat < 0, -1);

		int iLng = m_lng.getIdx(p.y);
		IF__(iLng < 0, -1);

		return iAlt * m_nLL + iLat * m_lng.m_nDiv + iLng;
	}

	GEOGRID_CELL GeoGrid::getCell(int32_t cIdx)
	{
		GEOGRID_CELL gC;
		IF__(m_nCell < 0, gC);
		IF__(cIdx < 0, gC);

		int iAlt = cIdx / m_nLL;
		int iLat = (cIdx - iAlt * m_nLL) / m_lng.m_nDiv;
		int iLng = (cIdx - iAlt * m_nLL - iLat * m_lng.m_nDiv);

		gC.m_vAlt = m_alt.getCell(iAlt);
		gC.m_vLat = m_lat.getCell(iLat);
		gC.m_vLng = m_lng.getCell(iLng);
		gC.calcCenter();

		return gC;
	}

	int GeoGrid::getNcell(void)
	{
		return m_nCell;
	}

	void GeoGrid::getCoverage(const vDouble4 &pCover, int *pCidx, float *pCoverage)
	{
	}

	void GeoGrid::console(void *pConsole)
	{
		this->BASE::console(pConsole);
	}

}

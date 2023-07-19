/*
 * GeoGrid.h
 *
 *  Created on: July 19, 2023
 *      Author: yankai
 */

#ifndef OpenKAI_src_Navigation_GeoGrid_H_
#define OpenKAI_src_Navigation_GeoGrid_H_

#include "../Base/BASE.h"
#include "../Script/Kiss.h"

namespace kai
{
	struct GEOGRID_AXIS
	{
		vDouble2 m_vRegion;
		int m_nDiv;
		double m_k;
	};

	struct GEOGRID_CELL
	{
		vDouble2 m_vLat;
		vDouble2 m_vLng;
		vDouble2 m_vAlt;
		vDouble3 m_vCenter;

	};

	class GeoGrid : public BASE
	{
	public:
		GeoGrid(void);
		virtual ~GeoGrid();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual int check(void);
		virtual void console(void *pConsole);

		int gen(const GEOGRID_AXIS& lat, const GEOGRID_AXIS& lng, const GEOGRID_AXIS& alt);
		int getCellIdx(const vDouble3& p);
		GEOGRID_CELL getCell(uint32_t cIdx);

		void getCoverage(const vDouble4& pCover, int* pCidx, float* pCoverage);

	protected:
		GEOGRID_AXIS m_lat;
		GEOGRID_AXIS m_lng;
		GEOGRID_AXIS m_alt;

		int m_nCell;
		uint64_t m_gCode; // checksum of properties to determine the identity of grid systems
		

	};
}
#endif

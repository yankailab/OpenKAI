/*
 * GeoGrid.h
 *
 *  Created on: July 19, 2023
 *      Author: yankai
 */

#ifndef OpenKAI_src_Navigation_GeoGrid_H_
#define OpenKAI_src_Navigation_GeoGrid_H_

#include "../Base/BASE.h"
#include "../Module/Kiss.h"

namespace kai
{
	struct GEOGRID_AXIS
	{
		// params to be given
		vDouble2 m_vRegion = {0, 0};
		double m_kDiv = 0.0;
		int m_nDiv = 0;

		// vars to be generated
		double m_wAvr = 0;
		double m_wK = 0;
		int m_iMid = 0;

		bool gen(void)
		{
			IF_F(m_nDiv <= 0);
			IF_F(m_vRegion.x >= m_vRegion.y);
			IF_F(abs(m_kDiv) >= 1.0 / (m_nDiv / 2));

			m_wAvr = m_vRegion.len() / m_nDiv;
			m_wK = m_wAvr * m_kDiv;
			int m_iMid = m_nDiv / 2;
		}

		int getIdx(double v)
		{
			IF__(!m_vRegion.bInside(v), -1);

			int iA = 0;
			int iB = m_nDiv;
			int i = m_iMid;
			while (1)
			{
				vDouble2 vC = getCell(i);
				IF__(vC.bInside(v), i);

				if (v < vC.x)
					iB = i;
				else
					iA = i;

				i = (iA + iB) / 2;
			}
		}

		vDouble2 getCell(int i)
		{
			vDouble2 vC;
			vC.x = m_vRegion.x + (m_wAvr * i) + (m_wK * (i - m_iMid));
			vC.y = vC.x + m_wAvr + (m_wK * (i + 1 - m_iMid));

			return vC;
		}
	};

	struct GEOGRID_CELL
	{
		vDouble2 m_vLat;
		vDouble2 m_vLng;
		vDouble2 m_vAlt;
		vDouble3 m_vCenter;

		vDouble3 calcCenter(void)
		{
			m_vCenter.x = m_vLat.mid();
			m_vCenter.y = m_vLng.mid();
			m_vCenter.z = m_vAlt.mid();
		}
	};

	class GeoGrid : public BASE
	{
	public:
		GeoGrid(void);
		virtual ~GeoGrid();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int check(void);
		virtual void console(void *pConsole);

		int gen(const GEOGRID_AXIS &lat, const GEOGRID_AXIS &lng, const GEOGRID_AXIS &alt);
		int32_t getCellIdx(const vDouble3 &p); // lat, lng, alt
		GEOGRID_CELL getCell(int32_t cIdx);
		int getNcell(void);

		void getCoverage(const vDouble4 &pCover, int *pCidx, float *pCoverage);

	protected:
		GEOGRID_AXIS m_alt;
		GEOGRID_AXIS m_lat;
		GEOGRID_AXIS m_lng;

		int m_nCell;
		int m_nLL; // number of cells per altitude layer
		uint64_t m_gCode; // checksum of properties to determine the identity of grid systems
	};
}
#endif

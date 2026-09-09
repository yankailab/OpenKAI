#ifndef OpenKAI_src_3D_Grid__OctreeGrid_H_
#define OpenKAI_src_3D_Grid__OctreeGrid_H_

#include "_OctreeBase.h"
#include "../PointCloud/_PointCloud.h"
#include "../../Filter/Median.h"
#include "../../Primitive/UUID128.h"

#define OCTGRID_MAX_LEVEL 40

namespace kai
{
	struct OCTGRID_PCL_CELL
	{
		UUID128 m_ID = 0;
		/*
		cell ID format:
		128 bit width from MSB to LSB
		[2 bit] 0
		[3 bit][3bit]... each 3-bit fragment correspondent to its cell index at the Level from 0 to 39, 40 levels at most (3 bit x 40 = 120 bit)
		[6 bit] max level valid in the ID
		*/

		int m_nP = 0;
		vFloat3 m_vC = {1, 1, 1}; // default color
		uint64_t m_tStamp = 0;	  // last updated time stamp

		void clear(void)
		{
			m_ID = {0, 0};
			m_nP = 0;
			m_vC.set(1);
			m_tStamp = 0;
		}
	};

	class _OctreeGrid : public _OctreeBase
	{
	public:
		_OctreeGrid();
		virtual ~_OctreeGrid();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual bool check(void);

		// grid
		virtual OCTGRID_PCL_CELL *addCellPoint(const GEOMETRY_POINT &gP, const uint64_t &tNow, int nMaxLevTo = -1, bool bAdd = true);
		virtual OCTGRID_PCL_CELL *getCell(const vFloat3 &vP, int nMaxLevTo = -1);
		virtual OCTGRID_PCL_CELL *getCell(const UUID128 &id);

		// drawing
		virtual int get(GEOMETRY_RINGBUF<GEOMETRY_POINT> *pOut, uint64_t tExpire = 0);
		virtual int get(GEOMETRY_RINGBUF<GEOMETRY_LINE> *pOut, uint64_t tExpire = 0);

	protected:
		// data
		virtual void updatePoint(void);
		virtual void deleteExpiredCells(void);

		// drawing
		virtual void updateDrawAssets(void);

	private:
		virtual void updateGrid(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_OctreeGrid *)This)->update();
			return NULL;
		}

	protected:
		vFloat3 m_vPorigin;		 // cubic center of the root cell in local coordinate
		vFloat3 m_vRootCellSize; // root level cell size in meters
		int m_nMaxLevel;

		// data
		OCTREE_CELL<OCTGRID_PCL_CELL> *m_pCell; // root cell
		uint64_t m_dTexpireCell;				// remove cell if no point is coming by this duration

		// point cloud input
		vector<_GeometryBase *> m_vpGb;
		GEOMETRY_RINGBUF<GEOMETRY_POINT> m_grPt;
		uint64_t m_dTexpirePCL;

		// generated line for grid visualization
		int m_nMaxLines;
		GEOMETRY_RINGBUF<GEOMETRY_LINE> m_lnCellOcc;
		vFloat3 m_vColCellOcc;
	};

}
#endif

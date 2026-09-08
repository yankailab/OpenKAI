#ifndef OpenKAI_src_3D_Grid__OctreeGrid_H_
#define OpenKAI_src_3D_Grid__OctreeGrid_H_

#include "_OctreeBase.h"
#include "../PointCloud/_PointCloud.h"
#include "../../Filter/Median.h"

namespace kai
{
	struct OCT_PCL_CELL
	{
		int m_nP = 0;
		vFloat3 m_vC = {1, 1, 1}; // default color
		uint64_t m_tStamp = 0;	  // last updated time stamp

		void clear(void)
		{
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

		// drawing
		virtual int get(GEOMETRY_RINGBUF<GEOMETRY_POINT> *pOut, uint64_t tExpire = 0);
		virtual int get(GEOMETRY_RINGBUF<GEOMETRY_LINE> *pOut, uint64_t tExpire = 0);

	protected:
		// data
		virtual void updatePoint(void);

		// grid
		virtual OCT_PCL_CELL *getCell(const vFloat3 &vP, bool bAdd = true);

		virtual void deleteExpiredCells(void);
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
		vFloat3 m_vPorigin;	 // cubic center of the root cell in local coordinate
		vFloat3 m_vRootCellSize; // root level cell size in meters
		int m_nMaxLevel;

		// data
		OCTREE_CELL<OCT_PCL_CELL> *m_pCell; // root cell
		uint64_t m_dTexpireCell;			// remove cell if no point is coming by this duration

		// point cloud input
		vector<_GeometryBase *> m_vpGb;
		GEOMETRY_RINGBUF<GEOMETRY_POINT> m_grPt;
		uint64_t m_dTexpirePcl;

		// generated line for grid visualization
		GEOMETRY_RINGBUF<GEOMETRY_LINE> m_lnCellOcc;
		vFloat3 m_vColCellOcc;
		int m_nMaxLines;
	};

}
#endif

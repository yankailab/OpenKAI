#ifndef OpenKAI_src_3D_Grid__OctreeGrid_H_
#define OpenKAI_src_3D_Grid__OctreeGrid_H_

#include "_OctreeBase.h"
#include "../PointCloud/_PointCloud.h"
#include "../../Filter/Median.h"
#include "OctreeGridCells.h"
#include <mutex>

namespace kai
{
	struct OCTGRID_PCL_CELL
	{
		/*
		cell ID format:
		128 bit width from MSB to LSB
		[2 bit] 0
		[3 bit][3bit]... each 3-bit fragment correspondent to its cell index at the Level from 0 to 39, 40 levels at most (3 bit x 40 = 120 bit)
		[6 bit] cell depth (0 = root, 40 = deepest); unused path segments are zero
		*/
		UUID128 m_ID = 0;

		int m_nP = 0;
		vFloat4 m_vC = {1, 1, 1, 1}; // default color
		uint64_t m_tStamp = 0;		 // last updated time stamp

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
		virtual void console(void *pConsole);
		virtual void console(const json &j, void *pJSONbase);

		// config
		// Load after init, with grid updates stopped; restoring a new root clears old occupancy.
		virtual bool loadConfig(json *pJ = nullptr, string fName = "");
		virtual bool saveConfig(json &j, string fName = "");

		// grid
		virtual OCTGRID_PCL_CELL *addCellPoint(const GEOMETRY_POINT &gP, const uint64_t &tNow, int nMaxLevTo = -1, bool bAdd = true);
		virtual OCTGRID_PCL_CELL *getCell(const vFloat3 &vP, int nMaxLevTo = -1);
		virtual OCTGRID_PCL_CELL *getCell(const UUID128 &id);
		virtual const vector<UUID128>& getSelectedCells(void);

		// drawing
		virtual int get(GEOMETRY_RINGBUF<GEOMETRY_POINT> *pOut, uint64_t tExpire = 0);
		virtual int get(GEOMETRY_RINGBUF<GEOMETRY_LINE> *pOut, uint64_t tExpire = 0);
		virtual int get(OCTGRID_CELLS *pOut, uint64_t tExpire = 0, size_t nMaxCells = SIZE_MAX);

	protected:
		// data
		virtual void updatePoint(void);
		virtual void deleteExpiredCells(void);

		// drawing
		virtual void updateDrawAssets(void);

	private:
		json selectedCellsJSON(const char *idsKey);
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
		int m_nMaxLevel = 4;

		// data
		OCTREE_CELL<OCTGRID_PCL_CELL> *m_pCell = nullptr; // root cell
		uint64_t m_dTexpireCell = 0;				// remove cell if no point is coming by this duration

		// point cloud input
		vector<_GeometryBase *> m_vpGb;
		GEOMETRY_RINGBUF<GEOMETRY_POINT> m_grPt;
		uint64_t m_dTexpirePCL = 0;

		// Compact published snapshot; geometry is constructed by viewers.
		int m_nMaxCells = 100000 / 12;
		OCTGRID_CELLS m_cells;
		vector<OCTGRID_CELL> m_buildCells;
		// Serializes live root changes with grid updates; acquire before m_cellsMutex.
		std::mutex m_gridMutex;
		std::mutex m_cellsMutex;
		vFloat4 m_vColCellOcc;
		bool m_bColCellOcc = false;

		// Selected cells, guarded by m_cellsMutex after initialization.
		vector<UUID128> m_vSelectedCells;
	};

}
#endif

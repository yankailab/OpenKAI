#ifndef OpenKAI_src_3D_Grid__SelectableOctGrid_H_
#define OpenKAI_src_3D_Grid__SelectableOctGrid_H_

#include "_OctreeGrid.h"
#include "OctreeGridCells.h"

namespace kai
{
	// Adds viewer snapshots, persistent selection and browser commands to the grid.
	class _SelectableOctGrid : public _OctreeGrid
	{
	public:
		_SelectableOctGrid();
		virtual ~_SelectableOctGrid();

		bool init(const json &j) override;
		bool link(const json &j, ModuleMgr *pM) override;
		bool start(void) override;
		bool check(void) override;
		void console(void *pConsole) override;
		void console(const json &j, void *pJSONbase) override;

		// Restoring a different root clears occupancy before publishing its header.
		bool loadConfig(json *pJ = nullptr, string fName = "") override;
		bool saveConfig(json &j, string fName = "") override;
		virtual const vector<UUID128> &getSelectedCells(void);

		using _OctreeGrid::get;
		virtual int get(OCTGRID_CELLS *pOut, uint64_t tExpire = 0, size_t nMaxCells = SIZE_MAX);

	protected:
		void updateGrid(void) override;
		// Called with m_gridMutex held, or with grid updates stopped.
		virtual void updateDrawAssets(void);

	private:
		json selectedCellsJSON(const char *idsKey);
		void update(void) override;
		static void *getUpdate(void *This)
		{
			((_SelectableOctGrid *)This)->update();
			return NULL;
		}

	protected:
		// Compact published snapshot; viewers construct the geometry.
		int m_nMaxCells = 100000 / 12;
		OCTGRID_CELLS m_cells;
		vector<OCTGRID_CELL> m_buildCells;
		int m_nPminBuild = 1;
		Vector4f m_vColCellOcc = {1, 1, 1, 1};
		bool m_bColCellOcc = false;

		// Acquire m_gridMutex first when both locks are needed.
		std::mutex m_cellsMutex;
		vector<UUID128> m_vSelectedCells;
	};

}
#endif

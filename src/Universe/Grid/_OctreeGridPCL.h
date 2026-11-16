#ifndef OpenKAI_src_Universe_Grid__OctreeGridPCL_H_
#define OpenKAI_src_Universe_Grid__OctreeGridPCL_H_

#include <mutex>
#include "../../DataObject/PCLframe.h"
#include "_OctreeGridBase.h"

namespace kai
{
	struct OCTGRID_CELL_PCL
	{
		int m_nP = 0;
		Vector4f m_vC = {1, 1, 1, 1}; // default color
		uint64_t m_tStamp = 0;		  // last updated time stamp

		void clear(void)
		{
			m_nP = 0;
			m_vC.setConstant(1);
			m_tStamp = 0;
		}
	};

	class _OctreeGridPCL : public _OctreeGridBase
	{
		friend class _SelectableOctGridPCL;

	public:
		_OctreeGridPCL();
		virtual ~_OctreeGridPCL();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(InstanceMgr *pM) override;
		virtual bool start(void);
		virtual bool check(void);

		// Selection belongs to the runtime grid, independently of any editor/viewer.
		virtual vector<UUID128> getSelectedCells(void);
		virtual bool bSelectedOccupied(float threshold);

		// Return the spatial cell; point data is accessed through getT().
		using _OctreeGridBase::getCell;
		virtual OCTGRID_CELL<OCTGRID_CELL_PCL> *addCellPoint(const GEOMETRY_POINT &gP, const uint64_t &tNow, int nMaxLevTo = -1, bool bAdd = true);
		virtual OCTGRID_CELL<OCTGRID_CELL_PCL> *getCell(const Vector3f &vP, int nMaxLevTo = -1);
		virtual OCTGRID_CELL<OCTGRID_CELL_PCL> *getCell(const UUID128 &id);

	protected:
		// The update loop holds m_gridMutex across calculations and derived work.
		virtual void updateGrid(void);
		virtual void updatePoint(void);
		virtual void deleteExpiredCells(void);
		void resetPointInputs(void);

		// Caller holds m_gridMutex. Used by the editor's coherent Apply/Save operations.
		bool saveConfigLocked(bool bExport);
		json selectedCellIDsJSON(void) const;
		static bool readSelectionID(const json &j, UUID128 &id);
		static string writeSelectionID(const UUID128 &id);

	private:
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_OctreeGridPCL *)This)->update();
			return NULL;
		}

	protected:
		// data
		OCTGRID_CELL<OCTGRID_CELL_PCL> *m_pCell = nullptr; // root cell
		uint64_t m_dTexpireCell = 0;					  // remove cell if no point is coming by this duration

		struct PointInput
		{
			PCLframe *m_pFramein = nullptr;
			uint64_t m_tStamp = 0;
		};

		vector<string> m_vPCLframesIn;
		vector<PointInput> m_vPointInputs;
		vector<GEOMETRY_POINT> m_vInputPoints;
		uint64_t m_dTexpirePCL = 0;

		// Serializes point updates, selection access and editor root/config changes.
		std::mutex m_gridMutex;
		vector<UUID128> m_vSelectedCells;
	};

}
#endif

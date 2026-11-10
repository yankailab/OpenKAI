#ifndef OpenKAI_src_Universe_Grid__SelectableOctGrid_H_
#define OpenKAI_src_Universe_Grid__SelectableOctGrid_H_

#include "_OctreeGrid.h"
#include <array>
#include <cstddef>
#include <vector>

namespace kai
{
	// Byte arrays avoid UUID alignment padding. IDs are always little-endian,
	// low word first, both in memory and on the wire.
	struct SELECTABLE_OCTGRID_CELL
	{
		uint8_t m_ID[16] = {};
		uint8_t m_vC[4] = {0, 0, 0, 255}; // RGBA8

		void setID(const UUID128 &id)
		{
			for (unsigned i = 0; i < 16; ++i)
				m_ID[i] = uint8_t(id.m_uint64[i / 8] >> (8 * (i % 8)));
		}
		UUID128 id() const
		{
			UUID128 result(0);
			for (unsigned i = 0; i < 16; ++i)
				result.m_uint64[i / 8] |= uint64_t(m_ID[i]) << (8 * (i % 8));
			return result;
		}
	};
	static_assert(sizeof(SELECTABLE_OCTGRID_CELL) == 20, "Grid cells must occupy exactly 20 bytes");

	struct SELECTABLE_OCTGRID_HEADER
	{
		std::array<float, 3> m_vPorigin = {};			  // root center, not its minimum corner
		std::array<float, 3> m_vRootCellSize = {1, 1, 1}; // full XYZ extents
		uint32_t m_nMaxLevel = 0;
		uint64_t m_tStamp = 0; // monotonic nanoseconds at publication, shared by all records
	};

	struct SELECTABLE_OCTGRID_CELLS
	{
		SELECTABLE_OCTGRID_HEADER m_header;
		std::vector<SELECTABLE_OCTGRID_CELL> m_vCell; // complete snapshot, including occupied ancestors
	};

	// Decode without converting the 128-bit ID to a floating-point number.
	// Level 0 is the root; depth is at bits 125:120, child segments at 117, 114, ...
	inline bool octgridCellBox(const SELECTABLE_OCTGRID_HEADER &header, const UUID128 &id,
							   std::array<float, 3> &center, std::array<float, 3> &size)
	{
		unsigned depth = (id.m_uint64[1] >> 56) & 63;
		IF_F(header.m_nMaxLevel > OCTGRID_MAX_LEVEL || depth > header.m_nMaxLevel || id.m_uint64[1] >> 62);
		uint64_t high = id.m_uint64[1] & (UINT64_MAX >> 8), low = id.m_uint64[0];
		std::array<double, 3> c = {header.m_vPorigin[0], header.m_vPorigin[1], header.m_vPorigin[2]};
		std::array<double, 3> s = {header.m_vRootCellSize[0], header.m_vRootCellSize[1], header.m_vRootCellSize[2]};
		for (unsigned level = 0; level < depth; ++level)
		{
			const unsigned child = (high >> 53) & 7;
			for (unsigned axis = 0; axis < 3; ++axis)
			{
				c[axis] += s[axis] * ((child & (4 >> axis)) ? 0.25 : -0.25);
				s[axis] *= 0.5;
			}
			high = (high << 3) | (low >> 61);
			low <<= 3;
		}
		// Consumed path bits may remain in the high reserved/depth byte.
		IF_F((high & (UINT64_MAX >> 8)) || low);
		for (unsigned axis = 0; axis < 3; ++axis)
		{
			center[axis] = float(c[axis]);
			size[axis] = float(s[axis]);
		}
		return true;
	}


	// Adds viewer snapshots, persistent selection and browser commands to the grid.
	class _SelectableOctGrid : public _OctreeGrid
	{
	public:
		_SelectableOctGrid();
		virtual ~_SelectableOctGrid();

		bool loadConfig(void) override;
		bool start(void) override;
		void jsonCmd(const json &j, void *pJSONbase) override;

		// Persist a coherent grid configuration and selection.
		bool saveConfig(bool bExport) override;
		virtual const vector<UUID128> &getSelectedCells(void);
		// Checks live counts while holding both the grid and selection locks.
		virtual bool bSelectedOccupied(float threshold);

		// Cell snapshots are the grid's only stream output.
		virtual int get(SELECTABLE_OCTGRID_CELLS *pOut, uint64_t tExpire = 0, size_t nMaxCells = SIZE_MAX);

	protected:
		void updateGrid(void) override;
		// Called with m_gridMutex held, or with grid updates stopped.
		virtual void updateDrawAssets(void);

	private:
		// Called with m_gridMutex held; selection snapshots also take m_cellsMutex.
		bool saveConfigLocked(bool bExport);
		bool applyGridConfig(const json &j, string &error);
		bool applyCellSelection(const json &j, string &error);
		json gridRootJSON(void) const;
		json gridConfigJSON(void) const;
		json cellSelectionJSON(void);
		json selectedCellIDsJSON(void);
		void update(void) override;
		static void *getUpdate(void *This)
		{
			((_SelectableOctGrid *)This)->update();
			return NULL;
		}

	protected:
		// Compact published snapshot; viewers construct the geometry.
		int m_nMaxCells = 8333;
		SELECTABLE_OCTGRID_CELLS m_cells;
		vector<SELECTABLE_OCTGRID_CELL> m_buildCells;

		// Publish cells only when their point count exceeds this threshold (1 requires at least 2 points).
		int m_nPminBuild = 1;
		Vector4f m_vColCellOcc = {1, 1, 1, 1};
		bool m_bColCellOcc = false;

		// Acquire m_gridMutex first when both locks are needed.
		std::mutex m_cellsMutex;
		vector<UUID128> m_vSelectedCells;
	};

}
#endif

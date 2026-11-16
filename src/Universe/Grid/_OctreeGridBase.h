#ifndef OpenKAI_src_Universe_Grid__OctreeGridBase_H_
#define OpenKAI_src_Universe_Grid__OctreeGridBase_H_

#include "../../Base/_ModuleBase.h"
#include "../../Primitive/UUID128.h"
#include <array>
#include <cmath>

#define N_OCT 8

namespace kai
{
	constexpr int OCTGRID_MAX_LEVEL = 40;

	// IDs describe spatial cells independently of the payload stored in them.
	bool octgridValidCellID(const UUID128 &id, int maxLevel = OCTGRID_MAX_LEVEL);
	unsigned octgridCellChild(const UUID128 &id, unsigned level);
	bool octgridChildID(const UUID128 &parent, uint8_t child, UUID128 &id);
	bool octgridCellBox(const std::array<float, 3> &origin, const std::array<float, 3> &rootSize,
					   int maxLevel, const UUID128 &id, std::array<float, 3> &center, std::array<float, 3> &size);

	template <typename T>
	struct OCTGRID_CELL
	{
		/*
		128-bit cell ID, from MSB to LSB:
		[127:126] two reserved bits, zero
		[125:120] six-bit depth (0 = root, 40 = deepest)
		[119:0] forty three-bit child indices, starting at bits 119:117;
		        unused path segments are zero
		*/
		UUID128 m_ID = 0;
		T *m_pT = nullptr;
		OCTGRID_CELL *m_pParent = nullptr;
		OCTGRID_CELL *m_pChild[N_OCT] = {};
		// Child index: 4bX+2bY+bZ; each bit is 0 for the negative half, 1 for positive.

		OCTGRID_CELL *addChild(uint8_t iC)
		{
			IF_N(iC >= N_OCT);

			OCTGRID_CELL *pC = m_pChild[iC];
			IF__(pC, pC);

			UUID128 id;
			IF_N(!octgridChildID(m_ID, iC, id));
			pC = new OCTGRID_CELL();
			pC->m_ID = id;
			pC->m_pParent = this;
			m_pChild[iC] = pC;

			return pC;
		}

		OCTGRID_CELL *getChild(uint8_t iC)
		{
			IF_N(iC >= N_OCT);

			return m_pChild[iC];
		}

		void deleteChild(uint8_t iC)
		{
			IF_(iC >= N_OCT);

			OCTGRID_CELL* pC = m_pChild[iC];
			NULL_(pC);

			pC->release();
			delete pC;
			m_pChild[iC] = nullptr;
		}

		int getLevel(void) const
		{
			return int((m_ID.m_uint64[1] >> 56) & 63);
		}

		T *addT(void)
		{
			if (m_pT == nullptr)
				m_pT = new T();

			return m_pT;
		}

		T *getT(void)
		{
			return m_pT;
		}

		const T *getT(void) const
		{
			return m_pT;
		}

		// Clear payload and descendants while preserving this cell's identity and parent.
		void release(void)
		{
			DEL(m_pT);

			for (int i = 0; i < N_OCT; i++)
			{
				OCTGRID_CELL *pC = m_pChild[i];
				IF_CONT(pC == nullptr);

				pC->release();
				delete pC;
				m_pChild[i] = nullptr;
			}
		}
	};

	// Grid geometry and typed cell traversal; subclasses own their payloads and roots.
	class _OctreeGridBase : public _ModuleBase
	{
	public:
		_OctreeGridBase();
		virtual ~_OctreeGridBase();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(InstanceMgr *pM) override;
		virtual bool start(void);
		virtual bool check(void);

		// Look up structural cells, including cells without a payload. pRoot is level 0.
		template <typename T>
		OCTGRID_CELL<T> *getCell(OCTGRID_CELL<T> *pRoot, const UUID128 &id) const;

		template <typename T>
		OCTGRID_CELL<T> *getCell(OCTGRID_CELL<T> *pRoot, const Vector3f &position, int maxLevel = -1) const;

	protected:
		virtual void updateOctree(void);

		// Visit root through target, optionally creating nodes. Payload policy stays in the visitor.
		template <typename T, typename Visitor>
		OCTGRID_CELL<T> *visitCells(OCTGRID_CELL<T> *pRoot, const Vector3f &position,
			int maxLevel, bool add, Visitor visit) const;

	private:
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_OctreeGridBase *)This)->update();
			return NULL;
		}

	protected:
		Vector3f m_vPorigin = Vector3f::Zero(); // root center in local coordinates
		Vector3f m_vRootCellSize = {10, 10, 10}; // full root XYZ extents in meters
		int m_nMaxLevel = 4;
	};

	template <typename T>
	OCTGRID_CELL<T> *_OctreeGridBase::getCell(OCTGRID_CELL<T> *pRoot, const UUID128 &id) const
	{
		IF_N(!octgridValidCellID(id, m_nMaxLevel));
		OCTGRID_CELL<T> *pCell = pRoot;
		const unsigned depth = (id.m_uint64[1] >> 56) & 63;
		for (unsigned level = 0; pCell && level < depth; ++level)
			pCell = pCell->getChild(octgridCellChild(id, level));
		IF_N(!pCell || pCell->m_ID.m_uint64[0] != id.m_uint64[0] ||
			pCell->m_ID.m_uint64[1] != id.m_uint64[1]);
		return pCell;
	}

	template <typename T>
	OCTGRID_CELL<T> *_OctreeGridBase::getCell(OCTGRID_CELL<T> *pRoot, const Vector3f &position, int maxLevel) const
	{
		return visitCells(pRoot, position, maxLevel, false, [](OCTGRID_CELL<T> *) { return true; });
	}

	template <typename T, typename Visitor>
	OCTGRID_CELL<T> *_OctreeGridBase::visitCells(OCTGRID_CELL<T> *pRoot, const Vector3f &position,
		int maxLevel, bool add, Visitor visit) const
	{
		NULL_N(pRoot);
		const float *point = position.data();
		const float *origin = m_vPorigin.data(), *rootSize = m_vRootCellSize.data();
		float center[3], size[3];
		for (unsigned axis = 0; axis < 3; ++axis)
		{
			IF_N(!std::isfinite(point[axis]) ||
				std::fabs(point[axis] - origin[axis]) > rootSize[axis] * 0.5f);
			center[axis] = origin[axis];
			size[axis] = rootSize[axis];
		}

		if (maxLevel < 0 || maxLevel > m_nMaxLevel)
			maxLevel = m_nMaxLevel;

		OCTGRID_CELL<T> *cell = pRoot;
		for (int level = 0; level <= maxLevel; ++level)
		{
			IF_N(!visit(cell));
			IF__(level == maxLevel, cell);
			const uint8_t child = (point[0] >= center[0] ? 4 : 0) |
				(point[1] >= center[1] ? 2 : 0) | (point[2] >= center[2] ? 1 : 0);
			cell = add ? cell->addChild(child) : cell->getChild(child);
			NULL_N(cell);
			for (unsigned axis = 0; axis < 3; ++axis)
			{
				center[axis] += size[axis] * ((child & (4 >> axis)) ? 0.25f : -0.25f);
				size[axis] *= 0.5f;
			}
		}

		return nullptr;
	}

}

#endif

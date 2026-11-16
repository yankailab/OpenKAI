#ifndef OpenKAI_src_Universe_Grid__OctreeGridBase_H_
#define OpenKAI_src_Universe_Grid__OctreeGridBase_H_

#include "../../Base/_ModuleBase.h"
#include "../../Primitive/UUID128.h"
#include <array>
#include <functional>

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

	// Payload-independent traversal interface; ownership remains with the typed grid.
	struct OCTGRID_CELL_BASE
	{
		/*
		128-bit cell ID, from MSB to LSB:
		[127:126] two reserved bits, zero
		[125:120] six-bit depth (0 = root, 40 = deepest)
		[119:0] forty three-bit child indices, starting at bits 119:117;
		        unused path segments are zero
		*/
		UUID128 m_ID = 0;
		virtual OCTGRID_CELL_BASE *addChild(uint8_t iC) = 0;
		virtual OCTGRID_CELL_BASE *getChild(uint8_t iC) = 0;
		int getLevel(void) const;

	protected:
		// Prevent deletion through the traversal interface.
		virtual ~OCTGRID_CELL_BASE() = default;
	};

	template <typename T>
	struct OCTGRID_CELL : public OCTGRID_CELL_BASE
	{
		T *m_pT = nullptr;
		OCTGRID_CELL *m_pParent = nullptr;
		OCTGRID_CELL *m_pChild[N_OCT] = {};
		// Child index: 4bX+2bY+bZ; each bit is 0 for the negative half, 1 for positive.

		OCTGRID_CELL *addChild(uint8_t iC) override
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

		OCTGRID_CELL *getChild(uint8_t iC) override
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
		OCTGRID_CELL_BASE *getCell(OCTGRID_CELL_BASE *pRoot, const UUID128 &id) const;
		OCTGRID_CELL_BASE *getCell(OCTGRID_CELL_BASE *pRoot, const Vector3f &position, int maxLevel = -1) const;

	protected:
		virtual void updateOctree(void);

		// Visit root through target, optionally creating nodes. An empty visitor only traverses.
		OCTGRID_CELL_BASE *visitCells(OCTGRID_CELL_BASE *pRoot, const Vector3f &position,
									 int maxLevel, bool add, const std::function<bool(OCTGRID_CELL_BASE *)> &visit) const;

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

}
#endif

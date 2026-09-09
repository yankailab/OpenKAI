#include "_OctreeGrid.h"
#include <algorithm>
#include <cmath>

namespace kai
{
	namespace
	{
		static vFloat3 vHalf(const vFloat3 &v)
		{
			return vFloat3(v.x * 0.5f, v.y * 0.5f, v.z * 0.5f);
		}

		static vFloat3 childCenter(const vFloat3 &vPc, const vFloat3 &vSize, uint8_t iC)
		{
			float kX = (iC & 4) ? 0.25f : -0.25f;
			float kY = (iC & 2) ? 0.25f : -0.25f;
			float kZ = (iC & 1) ? 0.25f : -0.25f;

			return vFloat3(vPc.x + vSize.x * kX,
						   vPc.y + vSize.y * kY,
						   vPc.z + vSize.z * kZ);
		}

		static uint8_t childIdx(const vFloat3 &vP, const vFloat3 &vPc)
		{
			uint8_t iC = 0;
			if (vP.x >= vPc.x)
				iC |= 4;
			if (vP.y >= vPc.y)
				iC |= 2;
			if (vP.z >= vPc.z)
				iC |= 1;

			return iC;
		}

		static bool bInCell(const vFloat3 &vP, const vFloat3 &vPc, const vFloat3 &vSize)
		{
			IF_F(fabs(vP.x - vPc.x) > vSize.x * 0.5f);
			IF_F(fabs(vP.y - vPc.y) > vSize.y * 0.5f);
			IF_F(fabs(vP.z - vPc.z) > vSize.z * 0.5f);

			return true;
		}

		static void updatePCLcell(OCTGRID_PCL_CELL *pCell, const GEOMETRY_POINT &p, uint64_t tNow)
		{
			NULL_(pCell);

			if (pCell->m_nP <= 0)
				pCell->m_vC = p.m_vC;
			else
				pCell->m_vC = (pCell->m_vC * (float)pCell->m_nP + p.m_vC) / (float)(pCell->m_nP + 1);

			pCell->m_nP++;
			pCell->m_tStamp = tNow;
		}

		static UUID128 calcCellIDSegment(uint8_t iC, int iLevel)
		{
			IF__(iLevel >= OCTGRID_MAX_LEVEL, 0);

			return UUID128(iC) << (3 * (OCTGRID_MAX_LEVEL - 1 - iLevel) + 6);
		}

		static bool bHasChild(OCTREE_CELL<OCTGRID_PCL_CELL> *pCell)
		{
			NULL_F(pCell);

			for (int i = 0; i < N_OCT; i++)
			{
				IF__(pCell->getChild(i), true);
			}

			return false;
		}

		static bool bKeepCell(OCTREE_CELL<OCTGRID_PCL_CELL> *pCell, uint64_t tExpire)
		{
			NULL_F(pCell);

			for (int i = 0; i < N_OCT; i++)
			{
				OCTREE_CELL<OCTGRID_PCL_CELL> *pChild = pCell->getChild(i);
				IF_CONT(!pChild);

				if (!bKeepCell(pChild, tExpire))
					pCell->deleteChild(i);
			}

			OCTGRID_PCL_CELL *pT = pCell->getT();
			if (pT && bExpired(pT->m_tStamp, tExpire))
				DEL(pCell->m_pT);

			IF__(pCell->getT(), true);
			return bHasChild(pCell);
		}

		static void addCellsRecursive(OCTREE_CELL<OCTGRID_PCL_CELL> *pCell,
			vector<OCTGRID_CELL> &out, size_t limit, const vFloat3 *pColor)
		{
			if (!pCell || out.size() >= limit) return;
			const auto *pT = pCell->getT();
			if (pT && pT->m_nP > 0 && pT->m_tStamp > 0)
			{
				OCTGRID_CELL cell;
				cell.setID(pT->m_ID);
				const auto &color = pColor ? *pColor : pT->m_vC;
				const float rgb[] = {color.x, color.y, color.z};
				for (int i = 0; i < 3; ++i)
					cell.m_vC[i] = uint8_t(std::clamp(std::isfinite(rgb[i]) ? rgb[i] : 1.f, 0.f, 1.f) * 255.f + 0.5f);
				out.push_back(cell);
			}
			for (int i = 0; i < N_OCT && out.size() < limit; ++i)
				addCellsRecursive(pCell->getChild(i), out, limit, pColor);
		}
	}

	_OctreeGrid::_OctreeGrid()
	{
		m_vPorigin.set(0);
		m_vRootCellSize.set(10);
		m_nMaxLevel = 4;

		m_pCell = nullptr;
		m_dTexpireCell = 0;
		m_dTexpirePCL = 0;

		m_nMaxCells = 100000 / 12;
		m_bColCellOcc = false;
		m_vColCellOcc.set(1);
	}

	_OctreeGrid::~_OctreeGrid()
	{
		if (m_pCell)
		{
			m_pCell->release();
			delete m_pCell;
		}

		m_grPt.release();
	}

	bool _OctreeGrid::init(const json &j)
	{
		IF_F(!this->_OctreeBase::init(j));

		jKv<float>(j, "vPorigin", m_vPorigin);
		jKv<float>(j, "vRootCellSize", m_vRootCellSize);
		jKv(j, "nMaxLevel", m_nMaxLevel);
		jKv(j, "dTexpireCell", m_dTexpireCell);
		jKv(j, "dTexpirePCL", m_dTexpirePCL);

		// Keep the old line budget as a compatibility default, in complete boxes.
		int nMaxLines = 100000;
		jKv(j, "nMaxLines", nMaxLines);
		m_nMaxCells = nMaxLines / 12;
		jKv(j, "nMaxCells", m_nMaxCells);
		m_bColCellOcc = jKv<float>(j, "vColCellOcc", m_vColCellOcc);

		IF_Le_F(nMaxLines <= 0 || m_nMaxCells < 0, "Invalid grid cell limit");
		IF_Le_F(m_nMaxLevel < 0 || m_nMaxLevel > OCTGRID_MAX_LEVEL, "Invalid nMaxLevel: " + i2str(m_nMaxLevel));
		IF_Le_F(!std::isfinite(m_vPorigin.x) || !std::isfinite(m_vPorigin.y) || !std::isfinite(m_vPorigin.z) ||
					!std::isfinite(m_vRootCellSize.x) || !std::isfinite(m_vRootCellSize.y) || !std::isfinite(m_vRootCellSize.z) ||
					m_vRootCellSize.x <= 0.0f ||
					m_vRootCellSize.y <= 0.0f ||
					m_vRootCellSize.z <= 0.0f,
				"Invalid vRootCellSize");

		int nP = 100000;
		jKv(j, "nP", nP);
		IF_Le_F(nP <= 0, "Invalid nP: " + i2str(nP));
		m_grPt.release();
		IF_Le_F(!m_grPt.alloc(nP), "Alloc failed with nP: " + i2str(nP));
		m_grPt.clear();

		m_cells.m_vCell.clear();
		m_cells.m_vCell.reserve(m_nMaxCells);
		m_buildCells.clear();
		m_buildCells.reserve(m_nMaxCells);
		m_cells.m_header = {{m_vPorigin.x, m_vPorigin.y, m_vPorigin.z},
			{m_vRootCellSize.x, m_vRootCellSize.y, m_vRootCellSize.z}, uint32_t(m_nMaxLevel), 0};

		if (m_pCell)
		{
			m_pCell->release();
			delete m_pCell;
			m_pCell = nullptr;
		}
		m_pCell = new OCTREE_CELL<OCTGRID_PCL_CELL>();
		NULL_F(m_pCell);

		return true;
	}

	bool _OctreeGrid::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_OctreeBase::link(j, pM));

		vector<string> vGn;
		jKv(j, "vGeometryBase", vGn);
		m_vpGb.clear();
		for (string n : vGn)
		{
			_GeometryBase *pG = (_GeometryBase *)(pM->findModule(n));
			IF_CONT(!pG);

			m_vpGb.push_back(pG);
		}

		return true;
	}

	bool _OctreeGrid::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _OctreeGrid::check(void)
	{
		return this->_OctreeBase::check();
	}

	void _OctreeGrid::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateGrid();
		}
	}

	void _OctreeGrid::updateGrid(void)
	{
		IF_(!check());

		updatePoint();
		deleteExpiredCells();
		updateDrawAssets();
	}

	void _OctreeGrid::updatePoint(void)
	{
		IF_(!check());
		NULL_(m_pCell);

		uint64_t tNow = getApproxTbootUs();
		uint64_t tExpire = 0;
		if (m_dTexpirePCL > 0)
			tExpire = (tNow > m_dTexpirePCL) ? tNow - m_dTexpirePCL : 0;

		for (_GeometryBase *pGb : m_vpGb)
		{
			m_grPt.clear();
			int nP = pGb->get(&m_grPt, tExpire);
			IF_CONT(nP <= 0);
			nP = small<int>(nP, m_grPt.nT());

			int i = 0;
			while (i < nP)
			{
				GEOMETRY_POINT *pGp = m_grPt.get(i++);
				if (pGp == nullptr)
					break;
				if (pGp->m_tStamp == 0)
					break;

				addCellPoint(*pGp, tNow, m_nMaxLevel, true);
			}
		}
	}

	OCTGRID_PCL_CELL *_OctreeGrid::addCellPoint(const GEOMETRY_POINT &gP, const uint64_t &tNow, int nMaxLevTo, bool bAdd)
	{
		NULL_N(m_pCell);
		IF_N(!bInCell(gP.m_vP, m_vPorigin, m_vRootCellSize));

		OCTREE_CELL<OCTGRID_PCL_CELL> *pCell = m_pCell;
		vFloat3 vPc = m_vPorigin;
		vFloat3 vSize = m_vRootCellSize;

		if (nMaxLevTo < 0 || nMaxLevTo > m_nMaxLevel)
			nMaxLevTo = m_nMaxLevel;

		UUID128 cellID(0); // path to the current node; low six bits hold its depth

		for (int iL = 0; iL <= nMaxLevTo; iL++)
		{
			uint8_t iC = childIdx(gP.m_vP, vPc);

			// PCL at this level
			OCTGRID_PCL_CELL *pPcl = bAdd ? pCell->addT() : pCell->getT();
			NULL_N(pPcl);
			pPcl->m_ID = cellID;
			pPcl->m_ID |= uint64_t(iL);
			updatePCLcell(pPcl, gP, tNow);
			IF__(iL >= nMaxLevTo, pPcl);

			// go for next level
			cellID |= calcCellIDSegment(iC, iL);
			OCTREE_CELL<OCTGRID_PCL_CELL> *pChild = pCell->getChild(iC);
			if (!pChild)
			{
				IF_N(!bAdd);
				IF_N(!(pChild = pCell->addChild(iC)));
				NULL_N(pChild);
			}

			vPc = childCenter(vPc, vSize, iC);
			vSize = vHalf(vSize);
			pCell = pChild;
		}

		return nullptr;
	}

	OCTGRID_PCL_CELL *_OctreeGrid::getCell(const vFloat3& vP, int nMaxLevTo)
	{
		NULL_N(m_pCell);
		IF_N(!bInCell(vP, m_vPorigin, m_vRootCellSize));

		OCTREE_CELL<OCTGRID_PCL_CELL> *pCell = m_pCell;
		vFloat3 vPc = m_vPorigin;
		vFloat3 vSize = m_vRootCellSize;

		if (nMaxLevTo < 0 || nMaxLevTo > m_nMaxLevel)
			nMaxLevTo = m_nMaxLevel;

		for (int iL = 0; iL <= nMaxLevTo; iL++)
		{
			uint8_t iC = childIdx(vP, vPc);

			// PCL at this level
			OCTGRID_PCL_CELL *pPcl = pCell->getT();
			NULL_N(pPcl);
			IF__(iL >= nMaxLevTo, pPcl);

			// go for next level
			OCTREE_CELL<OCTGRID_PCL_CELL> *pChild = pCell->getChild(iC);
			NULL_N(pChild);

			vPc = childCenter(vPc, vSize, iC);
			vSize = vHalf(vSize);
			pCell = pChild;
		}

		return nullptr;
	}

	OCTGRID_PCL_CELL *_OctreeGrid::getCell(const UUID128 &id)
	{
		NULL_N(m_pCell);

		const int nMaxLevTo = id.m_uint64[0] & 0x3f;
		IF_N(nMaxLevTo > OCTGRID_MAX_LEVEL || nMaxLevTo > m_nMaxLevel);
		IF_N(id.m_uint64[1] >> 62);

		OCTREE_CELL<OCTGRID_PCL_CELL> *pCell = m_pCell;
		uint64_t high = id.m_uint64[1];
		uint64_t low = id.m_uint64[0];
		for (int iL = 0; iL < nMaxLevTo; iL++)
		{
			const uint8_t iC = (high >> 59) & 7;
			pCell = pCell->m_pChild[iC];
			NULL_N(pCell);

			high = (high << 3) | (low >> 61);
			low <<= 3;
		}

		auto *pT = pCell->getT();
		if (!pT || pT->m_ID.m_uint64[0] != id.m_uint64[0] || pT->m_ID.m_uint64[1] != id.m_uint64[1])
			return nullptr;
		return pT;
	}

	void _OctreeGrid::deleteExpiredCells(void)
	{
		IF_(!m_pCell);
		IF_(m_dTexpireCell == 0);

		uint64_t tExpire = 0;
		if (m_dTexpireCell > 0)
		{
			uint64_t tNow = getApproxTbootUs();
			tExpire = (tNow > m_dTexpireCell) ? tNow - m_dTexpireCell : 0;
		}

		bKeepCell(m_pCell, tExpire);
	}

	void _OctreeGrid::updateDrawAssets(void)
	{
		IF_(!check());
		NULL_(m_pCell);

		// Traverse outside the publication lock so viewers only wait for a swap.
		m_buildCells.clear();
		addCellsRecursive(m_pCell, m_buildCells, m_nMaxCells, m_bColCellOcc ? &m_vColCellOcc : nullptr);
		std::lock_guard<std::mutex> lock(m_cellsMutex);
		m_cells.m_header = {{m_vPorigin.x, m_vPorigin.y, m_vPorigin.z},
			{m_vRootCellSize.x, m_vRootCellSize.y, m_vRootCellSize.z}, uint32_t(m_nMaxLevel), getApproxTbootUs()};
		m_cells.m_vCell.swap(m_buildCells);
	}

	int _OctreeGrid::get(OCTGRID_CELLS *pOut, uint64_t tExpire, size_t nMaxCells)
	{
		NULL__(pOut, 0);
		std::lock_guard<std::mutex> lock(m_cellsMutex);
		pOut->m_header = m_cells.m_header;
		const size_t n = bExpired(m_cells.m_header.m_tStamp, tExpire) ? 0 :
			std::min(nMaxCells, m_cells.m_vCell.size());
		pOut->m_vCell.assign(m_cells.m_vCell.begin(), m_cells.m_vCell.begin() + n);
		return int(n);
	}

	int _OctreeGrid::get(GEOMETRY_RINGBUF<GEOMETRY_POINT> *pOut, uint64_t tExpire)
	{
		// currently the grid itself does not draw points, so we just return 0
		return 0;
	}

	int _OctreeGrid::get(GEOMETRY_RINGBUF<GEOMETRY_LINE> *pOut, uint64_t tExpire)
	{
		// Kept for _GeometryBase compatibility. Occupied boxes use the cell API.
		return 0;
	}
}

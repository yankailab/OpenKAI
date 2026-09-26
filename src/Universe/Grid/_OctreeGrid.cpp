#include "_OctreeGrid.h"
#include <cmath>

namespace kai
{
	namespace
	{
		static Vector3f vHalf(const Vector3f &v)
		{
			return Vector3f(v.x() * 0.5f, v.y() * 0.5f, v.z() * 0.5f);
		}

		static Vector3f childCenter(const Vector3f &vPc, const Vector3f &vSize, uint8_t iC)
		{
			float kX = (iC & 4) ? 0.25f : -0.25f;
			float kY = (iC & 2) ? 0.25f : -0.25f;
			float kZ = (iC & 1) ? 0.25f : -0.25f;

			return Vector3f(vPc.x() + vSize.x() * kX,
							vPc.y() + vSize.y() * kY,
							vPc.z() + vSize.z() * kZ);
		}

		static uint8_t childIdx(const Vector3f &vP, const Vector3f &vPc)
		{
			uint8_t iC = 0;
			if (vP.x() >= vPc.x())
				iC |= 4;
			if (vP.y() >= vPc.y())
				iC |= 2;
			if (vP.z() >= vPc.z())
				iC |= 1;

			return iC;
		}

		static bool bInCell(const Vector3f &vP, const Vector3f &vPc, const Vector3f &vSize)
		{
			IF_F(fabs(vP.x() - vPc.x()) > vSize.x() * 0.5f);
			IF_F(fabs(vP.y() - vPc.y()) > vSize.y() * 0.5f);
			IF_F(fabs(vP.z() - vPc.z()) > vSize.z() * 0.5f);

			return true;
		}

		static void updatePCLcell(OCTGRID_PCL_CELL *pCell, const GEOMETRY_POINT &p, uint64_t tNow)
		{
			NULL_(pCell);

			if (pCell->m_nP <= 0)
			{
				pCell->m_vC = Vector4f(p.m_vC.x(), p.m_vC.y(), p.m_vC.z(), 1);
			}
			else
			{
				Vector4f color = Vector4f(p.m_vC.x(), p.m_vC.y(), p.m_vC.z(), 1);
				pCell->m_vC = (pCell->m_vC * (float)pCell->m_nP + color) / (float)(pCell->m_nP + 1);
			}

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
	}

	_OctreeGrid::_OctreeGrid()
	{
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

	bool _OctreeGrid::loadConfig(void)
	{
		IF_F(!this->_OctreeBase::loadConfig());
		const json &j = *m_pJ;

		jKv<float>(j, "vPorigin", m_vPorigin);
		jKv<float>(j, "vRootCellSize", m_vRootCellSize);
		jKv(j, "nMaxLevel", m_nMaxLevel);
		jKv(j, "dTexpireCell", m_dTexpireCell);
		jKv(j, "dTexpirePCL", m_dTexpirePCL);

		IF_Le_F(m_nMaxLevel < 0 || m_nMaxLevel > OCTGRID_MAX_LEVEL, "Invalid nMaxLevel: " + i2str(m_nMaxLevel));
		IF_Le_F(!std::isfinite(m_vPorigin.x()) || !std::isfinite(m_vPorigin.y()) || !std::isfinite(m_vPorigin.z()) ||
					!std::isfinite(m_vRootCellSize.x()) || !std::isfinite(m_vRootCellSize.y()) || !std::isfinite(m_vRootCellSize.z()) ||
					m_vRootCellSize.x() <= 0.0f ||
					m_vRootCellSize.y() <= 0.0f ||
					m_vRootCellSize.z() <= 0.0f,
				"Invalid vRootCellSize");

		int nP = 100000;
		jKv(j, "nP", nP);
		IF_Le_F(nP <= 0, "Invalid nP: " + i2str(nP));
		m_grPt.release();
		IF_Le_F(!m_grPt.alloc(nP), "Alloc failed with nP: " + i2str(nP));
		m_grPt.clear();

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

	bool _OctreeGrid::saveConfig(bool bExport)
	{
		IF_F(!_OctreeBase::saveConfig(false));

		json &j = *m_pJ;
		j["vPorigin"] = {m_vPorigin.x(), m_vPorigin.y(), m_vPorigin.z()};
		j["vRootCellSize"] = {m_vRootCellSize.x(), m_vRootCellSize.y(), m_vRootCellSize.z()};
		j["nMaxLevel"] = m_nMaxLevel;
		j["dTexpireCell"] = m_dTexpireCell;
		j["dTexpirePCL"] = m_dTexpirePCL;
		j["nP"] = m_grPt.m_nT;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _OctreeGrid::link(void)
	{
		NULL_F(m_pM);
		IF_F(!this->_OctreeBase::link());
		const json &j = *m_pJ;

		vector<string> vGn;
		jKv(j, "vGeometryBase", vGn);
		m_vpGb.clear();
		for (string n : vGn)
		{
			auto *pSource = static_cast<BASE *>(m_pM->findModule(n));
			IF_CONT(!pSource);
			auto *pG = dynamic_cast<_GeometryBase *>(pSource);
			IF_Le_F(!pG, "Grid input is not a geometry source: " + n);

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
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			std::lock_guard<std::mutex> gridLock(m_gridMutex);
			updateGrid();
		}
	}

	void _OctreeGrid::updateGrid(void)
	{
		IF_(!check());

		updatePoint();
		deleteExpiredCells();
	}

	void _OctreeGrid::updatePoint(void)
	{
		IF_(!check());
		NULL_(m_pCell);

		uint64_t tNow = getTns();
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
		Vector3f vPc = m_vPorigin;
		Vector3f vSize = m_vRootCellSize;

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

	OCTGRID_PCL_CELL *_OctreeGrid::getCell(const Vector3f &vP, int nMaxLevTo)
	{
		NULL_N(m_pCell);
		IF_N(!bInCell(vP, m_vPorigin, m_vRootCellSize));

		OCTREE_CELL<OCTGRID_PCL_CELL> *pCell = m_pCell;
		Vector3f vPc = m_vPorigin;
		Vector3f vSize = m_vRootCellSize;

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
		IF_N(id.m_uint64[1] >> 62); // verify if the header is 0x00b

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
		IF_N(!pT || pT->m_nP <= 0 || pT->m_tStamp == 0 ||
			 pT->m_ID.m_uint64[0] != id.m_uint64[0] || pT->m_ID.m_uint64[1] != id.m_uint64[1]);

		return pT;
	}

	void _OctreeGrid::deleteExpiredCells(void)
	{
		IF_(!m_pCell);
		IF_(m_dTexpireCell == 0);

		uint64_t tExpire = 0;
		if (m_dTexpireCell > 0)
		{
			uint64_t tNow = getTns();
			tExpire = (tNow > m_dTexpireCell) ? tNow - m_dTexpireCell : 0;
		}

		bKeepCell(m_pCell, tExpire);
	}

}

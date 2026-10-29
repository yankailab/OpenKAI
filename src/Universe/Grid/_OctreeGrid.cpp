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

			// This is the innermost loop for every point at every octree level.
			// Evaluate the three-channel mean directly: Eigen expressions here
			// dominate dense-cloud builds without compiler optimization.
			float *color = pCell->m_vC.data();
			const float *sample = p.m_vC.data();
			if (pCell->m_nP <= 0)
			{
				for (unsigned axis = 0; axis < 3; ++axis)
					color[axis] = sample[axis];
			}
			else
			{
				const float count = float(pCell->m_nP);
				const float nextCount = float(pCell->m_nP + 1);
				for (unsigned axis = 0; axis < 3; ++axis)
					color[axis] = (color[axis] * count + sample[axis]) / nextCount;
			}
			color[3] = 1.f;

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
		jKv(j, "vPCLframesIn", m_vPCLframesIn);

		IF_Le_F(m_nMaxLevel < 0 || m_nMaxLevel > OCTGRID_MAX_LEVEL, "Invalid nMaxLevel: " + i2str(m_nMaxLevel));
		IF_Le_F(!std::isfinite(m_vPorigin.x()) || !std::isfinite(m_vPorigin.y()) || !std::isfinite(m_vPorigin.z()) ||
					!std::isfinite(m_vRootCellSize.x()) || !std::isfinite(m_vRootCellSize.y()) || !std::isfinite(m_vRootCellSize.z()) ||
					m_vRootCellSize.x() <= 0.0f ||
					m_vRootCellSize.y() <= 0.0f ||
					m_vRootCellSize.z() <= 0.0f,
				"Invalid vRootCellSize");

		resetPointInputs();

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
		j["vPCLframesIn"] = m_vPCLframesIn;
		j.erase("nP");
		j.erase("vGeometryBase");

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _OctreeGrid::link(InstanceMgr *pM)
	{
		IF_F(!this->_OctreeBase::link(pM));
		m_vPointInputs.clear();
		m_vPointInputs.reserve(m_vPCLframesIn.size());
		for (const string &name : m_vPCLframesIn)
		{
			auto *pFramein = dynamic_cast<PCLframe *>(static_cast<DataObjBase *>(pM->findDataObject(name)));
			IF_Le_F(!pFramein, "Grid input is not a PCLframe: " + name);

			m_vPointInputs.push_back({pFramein, 0});
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

	void _OctreeGrid::resetPointInputs(void)
	{
		for (PointInput &input : m_vPointInputs)
		{
			input.m_tStamp = 0;
		}
	}

	void _OctreeGrid::updatePoint(void)
	{
		IF_(!check());
		NULL_(m_pCell);

		const uint64_t tNow = getTns();
		uint64_t tExpire = 0;
		if (m_dTexpirePCL > 0)
		{
			tExpire = (tNow > m_dTexpirePCL) ? tNow - m_dTexpirePCL : 0;
		}

		for (PointInput &input : m_vPointInputs)
		{
			if (input.m_pFramein->getTstamp() == input.m_tStamp)
			{
				continue;
			}
			const uint64_t stamp = input.m_pFramein->get(m_vInputPoints);
			if (stamp == input.m_tStamp)
			{
				continue;
			}
			input.m_tStamp = stamp;

			for (const GEOMETRY_POINT &point : m_vInputPoints)
			{
				const float *position = point.m_vP.data();
				if (bExpired(point.m_tStamp, tExpire) || !std::isfinite(position[0]) ||
					!std::isfinite(position[1]) || !std::isfinite(position[2]))
				{
					continue;
				}

				addCellPoint(point, tNow, m_nMaxLevel, true);
			}
		}
	}

	OCTGRID_PCL_CELL *_OctreeGrid::addCellPoint(const GEOMETRY_POINT &gP, const uint64_t &tNow, int nMaxLevTo, bool bAdd)
	{
		NULL_N(m_pCell);
		const float *point = gP.m_vP.data();
		const float *origin = m_vPorigin.data();
		const float *rootSize = m_vRootCellSize.data();
		float center[3], size[3];
		for (unsigned axis = 0; axis < 3; ++axis)
		{
			IF_N(std::fabs(point[axis] - origin[axis]) > rootSize[axis] * 0.5f);
			center[axis] = origin[axis];
			size[axis] = rootSize[axis];
		}

		OCTREE_CELL<OCTGRID_PCL_CELL> *pCell = m_pCell;

		if (nMaxLevTo < 0 || nMaxLevTo > m_nMaxLevel)
			nMaxLevTo = m_nMaxLevel;

		UUID128 cellID(0); // path to the current node; low six bits hold its depth

		for (int iL = 0; iL <= nMaxLevTo; iL++)
		{
			const uint8_t iC = (point[0] >= center[0] ? 4 : 0) |
				(point[1] >= center[1] ? 2 : 0) | (point[2] >= center[2] ? 1 : 0);

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

			for (unsigned axis = 0; axis < 3; ++axis)
			{
				center[axis] += size[axis] * ((iC & (4 >> axis)) ? 0.25f : -0.25f);
				size[axis] *= 0.5f;
			}
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

#include "_OctreeGridPCL.h"
#include <cmath>

namespace kai
{
	namespace
	{
		static void updatePCLcell(OCTGRID_CELL_PCL *pCell, const GEOMETRY_POINT &p, uint64_t tNow)
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

		static bool bHasChild(OCTGRID_CELL<OCTGRID_CELL_PCL> *pCell)
		{
			NULL_F(pCell);

			for (int i = 0; i < N_OCT; i++)
			{
				IF__(pCell->getChild(i), true);
			}

			return false;
		}

		static bool bKeepCell(OCTGRID_CELL<OCTGRID_CELL_PCL> *pCell, uint64_t tExpire)
		{
			NULL_F(pCell);

			for (int i = 0; i < N_OCT; i++)
			{
				OCTGRID_CELL<OCTGRID_CELL_PCL> *pChild = pCell->getChild(i);
				IF_CONT(!pChild);

				if (!bKeepCell(pChild, tExpire))
					pCell->deleteChild(i);
			}

			OCTGRID_CELL_PCL *pT = pCell->getT();
			if (pT && bExpired(pT->m_tStamp, tExpire))
				DEL(pCell->m_pT);

			IF__(pCell->getT(), true);
			return bHasChild(pCell);
		}
	}

	_OctreeGridPCL::_OctreeGridPCL()
	{
	}

	_OctreeGridPCL::~_OctreeGridPCL()
	{
		if (m_pCell)
		{
			m_pCell->release();
			delete m_pCell;
		}
	}

	bool _OctreeGridPCL::loadConfig(void)
	{
		IF_F(!this->_OctreeGridBase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "dTexpireCell", m_dTexpireCell);
		jKv(j, "dTexpirePCL", m_dTexpirePCL);
		jKv(j, "vPCLframesIn", m_vPCLframesIn);

		resetPointInputs();

		if (m_pCell)
		{
			m_pCell->release();
			delete m_pCell;
			m_pCell = nullptr;
		}
		m_pCell = new OCTGRID_CELL<OCTGRID_CELL_PCL>();
		NULL_F(m_pCell);

		return true;
	}

	bool _OctreeGridPCL::saveConfig(bool bExport)
	{
		IF_F(!_OctreeGridBase::saveConfig(false));

		json &j = *m_pJ;
		j["dTexpireCell"] = m_dTexpireCell;
		j["dTexpirePCL"] = m_dTexpirePCL;
		j["vPCLframesIn"] = m_vPCLframesIn;
		j.erase("nP");
		j.erase("vGeometryBase");

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _OctreeGridPCL::link(InstanceMgr *pM)
	{
		IF_F(!this->_OctreeGridBase::link(pM));
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

	bool _OctreeGridPCL::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _OctreeGridPCL::check(void)
	{
		return this->_OctreeGridBase::check();
	}

	void _OctreeGridPCL::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			std::lock_guard<std::mutex> gridLock(m_gridMutex);
			updateGrid();
		}
	}

	void _OctreeGridPCL::updateGrid(void)
	{
		IF_(!check());

		updatePoint();
		deleteExpiredCells();
	}

	void _OctreeGridPCL::resetPointInputs(void)
	{
		for (PointInput &input : m_vPointInputs)
		{
			input.m_tStamp = 0;
		}
	}

	void _OctreeGridPCL::updatePoint(void)
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

	OCTGRID_CELL<OCTGRID_CELL_PCL> *_OctreeGridPCL::addCellPoint(const GEOMETRY_POINT &gP, const uint64_t &tNow, int nMaxLevTo, bool bAdd)
	{
		return visitCells(m_pCell, gP.m_vP, nMaxLevTo, bAdd, [&](OCTGRID_CELL<OCTGRID_CELL_PCL> *cell)
		{
			OCTGRID_CELL_PCL *pPcl = bAdd ? cell->addT() : cell->getT();
			NULL_F(pPcl);
			updatePCLcell(pPcl, gP, tNow);
			return true;
		});
	}

	OCTGRID_CELL<OCTGRID_CELL_PCL> *_OctreeGridPCL::getCell(const Vector3f &vP, int nMaxLevTo)
	{
		return _OctreeGridBase::getCell(m_pCell, vP, nMaxLevTo);
	}

	OCTGRID_CELL<OCTGRID_CELL_PCL> *_OctreeGridPCL::getCell(const UUID128 &id)
	{
		return _OctreeGridBase::getCell(m_pCell, id);
	}

	void _OctreeGridPCL::deleteExpiredCells(void)
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

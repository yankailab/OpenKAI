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
		if (m_pT) m_pT->join();
		if (m_pCell)
		{
			m_pCell->release();
			delete m_pCell;
		}
	}

	bool _OctreeGridPCL::readSelectionID(const json &j, UUID128 &id)
	{
		IF_F(!j.is_string());
		const auto &s = j.get_ref<const string &>();
		IF_F(s.size() != 32);
		auto hex = [](char c) -> int
		{
			if (c >= '0' && c <= '9')
				return c - '0';
			if (c >= 'a' && c <= 'f')
				return c - 'a' + 10;
			if (c >= 'A' && c <= 'F')
				return c - 'A' + 10;
			return -1;
		};
		id = UUID128(0);
		for (size_t i = 0; i < 16; ++i)
		{
			const int high = hex(s[i * 2]), low = hex(s[i * 2 + 1]);
			IF_F(high < 0 || low < 0);
			id.m_uint64[i / 8] |= uint64_t(high * 16 + low) << (8 * (i % 8));
		}
		return true;
	}

	string _OctreeGridPCL::writeSelectionID(const UUID128 &id)
	{
		static const char hex[] = "0123456789abcdef";
		string result(32, '0');
		for (size_t i = 0; i < 16; ++i)
		{
			const uint8_t byte = uint8_t(id.m_uint64[i / 8] >> (8 * (i % 8)));
			result[i * 2] = hex[byte >> 4];
			result[i * 2 + 1] = hex[byte & 15];
		}
		return result;
	}

	json _OctreeGridPCL::selectedCellIDsJSON(void) const
	{
		json ids = json::array();
		for (const auto &id : m_vSelectedCells) ids.push_back(writeSelectionID(id));
		return ids;
	}

	vector<UUID128> _OctreeGridPCL::getSelectedCells(void)
	{
		std::lock_guard<std::mutex> gridLock(m_gridMutex);
		return m_vSelectedCells;
	}

	bool _OctreeGridPCL::bSelectedOccupied(float threshold)
	{
		std::lock_guard<std::mutex> gridLock(m_gridMutex);
		const uint64_t now = getTns();
		const uint64_t expire = m_dTexpireCell > 0 && now > m_dTexpireCell ? now - m_dTexpireCell : 0;
		for (const auto &id : m_vSelectedCells)
		{
			const auto *cell = getCell(id);
			const auto *pPcl = cell ? cell->getT() : nullptr;
			if (pPcl && pPcl->m_nPfiltered > 0 && pPcl->m_tStamp > 0 &&
				!bExpired(pPcl->m_tStamp, expire) && pPcl->m_nPfiltered > threshold)
				return true;
		}
		return false;
	}

	bool _OctreeGridPCL::loadConfig(void)
	{
		IF_F(!this->_OctreeGridBase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "dTexpireCell", m_dTexpireCell);
		jKv(j, "dTexpirePCL", m_dTexpirePCL);
		jKv(j, "nWfilMed", m_nWfilMed);
		jKv(j, "nWfilAvr", m_nWfilAvr);
		m_nWfilMed = std::max(1, m_nWfilMed);
		m_nWfilAvr = std::max(1, m_nWfilAvr);
		jKv(j, "vPCLframesIn", m_vPCLframesIn);

		m_vSelectedCells.clear();
		const json *ids = jK(j, "vSelectedCells");
		if (ids)
		{
			IF_Le_F(!ids->is_array(), "Invalid vSelectedCells");
			vector<UUID128> selected;
			selected.reserve(ids->size());
			for (const auto &value : *ids)
			{
				UUID128 id;
				IF_Le_F(!readSelectionID(value, id) || !octgridValidCellID(id, m_nMaxLevel),
						"Invalid selected cell ID in config");
				selected.push_back(id);
			}
			m_vSelectedCells.swap(selected);
		}

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
		std::lock_guard<std::mutex> gridLock(m_gridMutex);
		return saveConfigLocked(bExport);
	}

	bool _OctreeGridPCL::saveConfigLocked(bool bExport)
	{
		IF_F(!_OctreeGridBase::saveConfig(false));

		json &j = *m_pJ;
		j["dTexpireCell"] = m_dTexpireCell;
		j["dTexpirePCL"] = m_dTexpirePCL;
		j["nWfilMed"] = m_nWfilMed;
		j["nWfilAvr"] = m_nWfilAvr;
		j["vPCLframesIn"] = m_vPCLframesIn;
		j["vSelectedCells"] = selectedCellIDsJSON();
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
			if (!m_pT->bRun()) break;

			std::lock_guard<std::mutex> gridLock(m_gridMutex);
			updateGrid();
		}
	}

	void _OctreeGridPCL::updateGrid(void)
	{
		IF_(!check());

		updatePoint();
		deleteExpiredCells();
		updatePointFilters();
	}

	void _OctreeGridPCL::updatePointFilters(bool bReset)
	{
		auto updateFilters = [this, bReset](auto &&self, OCTGRID_CELL<OCTGRID_CELL_PCL> *cell) -> void
		{
			NULL_(cell);
			if (auto *pPcl = cell->getT())
			{
				if (bReset)
				{
					pPcl->m_filMed.init(m_nWfilMed);
					pPcl->m_filAvr.init(m_nWfilAvr);
				}
				pPcl->m_nPfiltered = static_cast<int>(pPcl->m_filAvr.update(pPcl->m_filMed.update(pPcl->m_nP)));
			}
			for (int i = 0; i < N_OCT; ++i)
				self(self, cell->getChild(i));
		};
		updateFilters(updateFilters, m_pCell);
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
			OCTGRID_CELL_PCL *pPcl = cell->getT();
			if (!pPcl && bAdd)
			{
				pPcl = cell->addT();
				pPcl->m_filMed.init(m_nWfilMed);
				pPcl->m_filAvr.init(m_nWfilAvr);
			}
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

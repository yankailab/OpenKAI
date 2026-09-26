#include "_SelectableOctGrid.h"
#include "../../Protocol/_JSONbase.h"
#include <algorithm>
#include <charconv>
#include <cmath>

namespace kai
{
	namespace
	{
		static bool readSelectionVector(const json *pJ, Vector3f &v)
		{
			IF_F(!pJ || !pJ->is_array() || pJ->size() != 3);
			const json &j = *pJ;
			for (size_t i = 0; i < 3; ++i)
			{
				IF_F(!j[i].is_string());
				const auto &s = j[i].get_ref<const string &>();
				// Parse the browser's ASCII numbers independently of the process locale.
				const auto result = std::from_chars(s.data(), s.data() + s.size(), v[i]);
				IF_F(result.ec != std::errc() || result.ptr != s.data() + s.size() || !std::isfinite(v[i]));
			}
			return true;
		}

		static bool readSelectionID(const json &j, UUID128 &id)
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
			OCTGRID_CELL cell;
			for (size_t i = 0; i < 16; ++i)
			{
				const int high = hex(s[i * 2]), low = hex(s[i * 2 + 1]);
				IF_F(high < 0 || low < 0);
				cell.m_ID[i] = uint8_t(high * 16 + low);
			}
			id = cell.id(); // Stream byte order: little-endian, low 64-bit word first.
			return true;
		}

		static string writeSelectionID(const UUID128 &id)
		{
			static const char hex[] = "0123456789abcdef";
			OCTGRID_CELL cell;
			cell.setID(id);
			string result(32, '0');
			for (size_t i = 0; i < 16; ++i)
			{
				result[i * 2] = hex[cell.m_ID[i] >> 4];
				result[i * 2 + 1] = hex[cell.m_ID[i] & 15];
			}
			return result;
		}

		static void addCellsRecursive(OCTREE_CELL<OCTGRID_PCL_CELL> *pCell,
									  int nPmin,
									  vector<OCTGRID_CELL> &out,
									  size_t limit,
									  const Vector4f *pColor)
		{
			if (!pCell || out.size() >= limit)
				return;
			const auto *pT = pCell->getT();
			if (pT && pT->m_nP > nPmin && pT->m_tStamp > 0)
			{
				OCTGRID_CELL cell;
				cell.setID(pT->m_ID);
				Vector4f color = pColor ? *pColor : pT->m_vC;
				if (!pColor)
					color.w() = 0.5f; // Preserve the temporary viewer alpha override.
				const float rgba[] = {color.x(), color.y(), color.z(), color.w()};
				for (int i = 0; i < 4; ++i)
					cell.m_vC[i] = uint8_t(std::clamp(std::isfinite(rgba[i]) ? rgba[i] : 1.f, 0.f, 1.f) * 255.f + 0.5f);
				out.push_back(cell);
			}
			for (int i = 0; i < N_OCT && out.size() < limit; ++i)
				addCellsRecursive(pCell->getChild(i), nPmin, out, limit, pColor);
		}
	}

	_SelectableOctGrid::_SelectableOctGrid()
	{
	}

	_SelectableOctGrid::~_SelectableOctGrid()
	{
	}

	bool _SelectableOctGrid::loadConfig(void)
	{
		IF_F(!this->_OctreeGrid::loadConfig());
		const json &j = *m_pJ;

		IF_Le_F(j.contains("nMaxLines"), "Use nMaxCells for grid publication limits");
		jKv(j, "nMaxCells", m_nMaxCells);
		m_bColCellOcc = jKv<float>(j, "vColCellOcc", m_vColCellOcc);

		IF_Le_F(m_nMaxCells < 0, "Invalid grid cell limit");

		jKv(j, "nPminBuild", m_nPminBuild);
		m_cells.m_vCell.clear();
		m_cells.m_vCell.reserve(m_nMaxCells);
		m_buildCells.clear();
		m_buildCells.reserve(m_nMaxCells);
		m_cells.m_header = {{m_vPorigin.x(), m_vPorigin.y(), m_vPorigin.z()},
							{m_vRootCellSize.x(), m_vRootCellSize.y(), m_vRootCellSize.z()},
							uint32_t(m_nMaxLevel),
							0};

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
				std::array<float, 3> center, extent;
				IF_Le_F(!readSelectionID(value, id) || !octgridCellBox(m_cells.m_header, id, center, extent),
						"Invalid selected cell ID in config");
				selected.push_back(id);
			}
			m_vSelectedCells.swap(selected);
		}
		return true;
	}

	bool _SelectableOctGrid::link(void)
	{
		return this->_OctreeGrid::link();
	}

	bool _SelectableOctGrid::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _SelectableOctGrid::check(void)
	{
		return this->_OctreeGrid::check();
	}

	void _SelectableOctGrid::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			std::lock_guard<std::mutex> gridLock(m_gridMutex);
			updateGrid();
		}
	}

	void _SelectableOctGrid::updateGrid(void)
	{
		this->_OctreeGrid::updateGrid();
		updateDrawAssets();
	}

	void _SelectableOctGrid::console(void *pConsole)
	{
		this->_OctreeGrid::console(pConsole);
	}

	bool _SelectableOctGrid::saveConfig(bool bExport)
	{
		IF_F(!_OctreeGrid::saveConfig(false));

		json &j = *m_pJ;
		j.update(selectedCellsJSON("vSelectedCells"));
		j["nMaxCells"] = m_nMaxCells;
		j["nPminBuild"] = m_nPminBuild;
		if (m_bColCellOcc)
		{
			j["vColCellOcc"] = {m_vColCellOcc.x(), m_vColCellOcc.y(), m_vColCellOcc.z(), m_vColCellOcc.w()};
		}
		else
		{
			j.erase("vColCellOcc");
		}

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	json _SelectableOctGrid::selectedCellsJSON(const char *idsKey)
	{
		Vector3f origin = Vector3f::Zero(), size = Vector3f::Zero();
		vector<UUID128> vCselected;

		{
			std::lock_guard<std::mutex> lock(m_cellsMutex);
			origin = m_vPorigin;
			size = m_vRootCellSize;
			vCselected = m_vSelectedCells;
		}

		json jG = json::object();
		jG["vPorigin"] = {origin.x(), origin.y(), origin.z()};
		jG["vRootCellSize"] = {size.x(), size.y(), size.z()};
		jG[idsKey] = json::array();
		for (const auto &id : vCselected)
			jG[idsKey].push_back(writeSelectionID(id));
		return jG;
	}

	const vector<UUID128> &_SelectableOctGrid::getSelectedCells(void)
	{
		return m_vSelectedCells;
	}

	void _SelectableOctGrid::updateDrawAssets(void)
	{
		IF_(!check());
		NULL_(m_pCell);

		// Traverse outside the publication lock so viewers only wait for a swap.
		m_buildCells.clear();
		addCellsRecursive(m_pCell, m_nPminBuild, m_buildCells, m_nMaxCells, m_bColCellOcc ? &m_vColCellOcc : nullptr);
		std::lock_guard<std::mutex> lock(m_cellsMutex);
		m_cells.m_header = {{m_vPorigin.x(), m_vPorigin.y(), m_vPorigin.z()},
							{m_vRootCellSize.x(), m_vRootCellSize.y(), m_vRootCellSize.z()},
							uint32_t(m_nMaxLevel),
							getTns()};
		m_cells.m_vCell.swap(m_buildCells);
	}

	int _SelectableOctGrid::get(OCTGRID_CELLS *pOut, uint64_t tExpire, size_t nMaxCells)
	{
		NULL__(pOut, 0);
		std::lock_guard<std::mutex> lock(m_cellsMutex);
		pOut->m_header = m_cells.m_header;
		const size_t n = bExpired(m_cells.m_header.m_tStamp, tExpire) ? 0 : std::min(nMaxCells, m_cells.m_vCell.size());
		pOut->m_vCell.assign(m_cells.m_vCell.begin(), m_cells.m_vCell.begin() + n);
		return int(n);
	}

	void _SelectableOctGrid::console(const json &j, void *pJSONbase)
	{
		_JSONbase *pJb = (_JSONbase *)pJSONbase;
		string cmd;
		IF_(!jKv(j, "cmd", cmd));

		if (cmd == "octGridCellSelect")
		{
			Vector3f origin = Vector3f::Zero(), size = Vector3f::Zero();
			bool sameHeader = readSelectionVector(jK(j, "vPorigin"), origin) &&
							  readSelectionVector(jK(j, "vRootCellSize"), size);
			{
				std::lock_guard<std::mutex> lock(m_cellsMutex);
				for (size_t i = 0; sameHeader && i < 3; ++i)
					sameHeader = origin[i] == m_vPorigin[i] && size[i] == m_vRootCellSize[i] && size[i] > 0;
			}

			const json *ids = jK(j, "cellIDs");
			bool bSuccess = sameHeader && ids && ids->is_array();
			vector<UUID128> vCselected;
			if (bSuccess)
			{
				const OCTGRID_HEADER header = {{origin.x(), origin.y(), origin.z()},
											   {size.x(), size.y(), size.z()},
											   uint32_t(m_nMaxLevel),
											   0};
				vCselected.reserve(ids->size());
				for (const auto &value : *ids)
				{
					UUID128 id;
					std::array<float, 3> center, extent;

					// Validate depth/reserved path bits without requiring current occupancy.
					if (!readSelectionID(value, id) || !octgridCellBox(header, id, center, extent))
					{
						bSuccess = false;
						break;
					}

					vCselected.push_back(id);
				}
			}

			{
				std::lock_guard<std::mutex> lock(m_cellsMutex);
				// A live config update may have completed while the IDs were decoded.
				sameHeader = sameHeader && origin == m_vPorigin && size == m_vRootCellSize;
				bSuccess = bSuccess && sameHeader;
				if (!sameHeader)
					m_vSelectedCells.clear();
				else if (bSuccess)
					m_vSelectedCells.swap(vCselected); // Commit only a completely valid list.
			}

			if (bSuccess)
			{
				bSuccess = saveConfig(true);
			}

			NULL_(pJb);
			json jr = json::object();
			jr["cmd"] = "octGridCellSelect";
			jr["bSuccess"] = bSuccess;
			pJb->sendJson(jr);
		}
		else if (cmd == "loadCellSelect")
		{
			NULL_(pJb);

			// Retrieval snapshots the current selection without
			// changing the running grid's root or occupancy.
			json jr = selectedCellsJSON("cellIDs");
			jr["cmd"] = "cellSelect";
			jr["module"] = getName();
			jr["nMaxLevel"] = m_nMaxLevel;
			for (const char *key : {"vPorigin", "vRootCellSize"})
				for (auto &coordinate : jr[key])
					coordinate = coordinate.dump();

			pJb->sendJson(jr);
		}
		else if (cmd == "setGridConfig")
		{
			Vector3f origin = Vector3f::Zero(), size = Vector3f::Zero();
			const bool bSuccess = readSelectionVector(jK(j, "vPorigin"), origin) &&
								  readSelectionVector(jK(j, "vRootCellSize"), size) && size.x() > 0 && size.y() > 0 && size.z() > 0;
			if (bSuccess)
			{
				std::lock_guard<std::mutex> gridLock(m_gridMutex);
				std::lock_guard<std::mutex> lock(m_cellsMutex);
				if (origin != m_vPorigin || size != m_vRootCellSize)
				{
					const OCTGRID_HEADER header = {{origin.x(), origin.y(), origin.z()},
												   {size.x(), size.y(), size.z()},
												   uint32_t(m_nMaxLevel),
												   getTns()};
					if (m_pCell)
						m_pCell->release(); // Clear the root's occupancy as well as all eight subtrees.
					m_buildCells.clear();
					m_cells.m_vCell.clear();
					m_vPorigin = origin;
					m_vRootCellSize = size;
					m_cells.m_header = header; // Publish an empty snapshot until the next rebuild.
				}
			}

			NULL_(pJb);
			pJb->sendJson(json{{"cmd", "setGridConfig"}, {"module", getName()}, {"bSuccess", bSuccess}});
		}
	}
}

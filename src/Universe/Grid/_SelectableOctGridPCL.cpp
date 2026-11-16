#include "_SelectableOctGridPCL.h"
#include "../../Protocol/_JSONbase.h"
#include <algorithm>
#include <charconv>
#include <cmath>

namespace kai
{
	namespace
	{
		template <typename T>
		static bool readConfigInteger(const json *pJ, T &value)
		{
			IF_F(!pJ || !pJ->is_string());
			const auto &s = pJ->get_ref<const string &>();
			IF_F(s.empty() || !std::all_of(s.begin(), s.end(), [](char c) { return c >= '0' && c <= '9'; }));
			const auto result = std::from_chars(s.data(), s.data() + s.size(), value);
			return result.ec == std::errc() && result.ptr == s.data() + s.size();
		}

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

		// Match the viewer's volume-preserving selection remap. Disjoint/nested
		// octree regions are deduplicated before testing their union for coverage.
		static vector<UUID128> remapSelectedCells(const SELECTABLE_OCTGRID_HEADER &oldHeader, const SELECTABLE_OCTGRID_HEADER &newHeader,
												 vector<UUID128> ids)
		{
			struct Region { std::array<double, 3> min, max; };
			vector<Region> regions;
			std::sort(ids.begin(), ids.end(), [](const UUID128 &a, const UUID128 &b)
			{
				const uint64_t ah = a.m_uint64[1] & (UINT64_MAX >> 8), bh = b.m_uint64[1] & (UINT64_MAX >> 8);
				if (ah != bh) return ah < bh;
				if (a.m_uint64[0] != b.m_uint64[0]) return a.m_uint64[0] < b.m_uint64[0];
				return a.m_uint64[1] < b.m_uint64[1];
			});
			UUID128 previous(0);
			bool havePrevious = false;
			for (const auto &id : ids)
			{
				const unsigned depth = id.m_uint64[1] >> 56, previousDepth = previous.m_uint64[1] >> 56;
				bool covered = havePrevious && previousDepth <= depth;
				for (unsigned level = 0; covered && level < previousDepth; ++level)
					covered = octgridCellChild(previous, level) == octgridCellChild(id, level);
				if (covered) continue;
				previous = id;
				havePrevious = true;
				std::array<double, 3> center, size;
				for (unsigned axis = 0; axis < 3; ++axis)
				{
					center[axis] = oldHeader.m_vPorigin[axis];
					size[axis] = oldHeader.m_vRootCellSize[axis];
				}
				for (unsigned level = 0; level < depth; ++level)
					for (unsigned axis = 0; axis < 3; ++axis)
					{
						center[axis] += size[axis] * ((octgridCellChild(id, level) & (4 >> axis)) ? .25 : -.25);
						size[axis] *= .5;
					}
				Region region;
				for (unsigned axis = 0; axis < 3; ++axis)
				{
					region.min[axis] = center[axis] - size[axis] * .5;
					region.max[axis] = center[axis] + size[axis] * .5;
				}
				regions.push_back(region);
			}

			vector<UUID128> selected;
			size_t nodes = 0;
			auto visit = [&](auto &&self, UUID128 id, const std::array<double, 3> &center,
							 const std::array<double, 3> &size, const vector<Region> &candidates) -> void
			{
				++nodes;
				double covered = 0;
				vector<Region> intersecting;
				for (const auto &region : candidates)
				{
					double overlap = 1;
					for (unsigned axis = 0; axis < 3; ++axis)
						overlap *= std::max(0., std::min(center[axis] + size[axis] * .5, region.max[axis]) -
											std::max(center[axis] - size[axis] * .5, region.min[axis]));
					if (overlap > 0)
					{
						covered += overlap;
						intersecting.push_back(region);
					}
				}
				if (covered == 0) return;
				const unsigned depth = id.m_uint64[1] >> 56;
				if (covered >= size[0] * size[1] * size[2] * (1 - 1e-12) || depth >= newHeader.m_nMaxLevel || nodes >= 100000)
				{
					selected.push_back(id);
					return;
				}
				for (unsigned child = 0; child < N_OCT; ++child)
				{
					UUID128 childID;
					if (!octgridChildID(id, child, childID)) continue;
					std::array<double, 3> childCenter, childSize;
					for (unsigned axis = 0; axis < 3; ++axis)
					{
						childCenter[axis] = center[axis] + size[axis] * ((child & (4 >> axis)) ? .25 : -.25);
						childSize[axis] = size[axis] * .5;
					}
					self(self, childID, childCenter, childSize, intersecting);
				}
			};
			visit(visit, UUID128(0), {newHeader.m_vPorigin[0], newHeader.m_vPorigin[1], newHeader.m_vPorigin[2]},
				  {newHeader.m_vRootCellSize[0], newHeader.m_vRootCellSize[1], newHeader.m_vRootCellSize[2]}, regions);
			return selected;
		}

		static void addCellsRecursive(OCTGRID_CELL<OCTGRID_CELL_PCL> *pCell,
									  int nPmin,
									  vector<SELECTABLE_OCTGRID_CELL> &out,
									  size_t limit,
									  const Vector4f *pColor)
		{
			if (!pCell || out.size() >= limit)
				return;
			const auto *pT = pCell->getT();
			if (pT && pT->m_nP > nPmin && pT->m_tStamp > 0)
			{
				SELECTABLE_OCTGRID_CELL cell;
				cell.setID(pCell->m_ID);
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

	_SelectableOctGridPCL::_SelectableOctGridPCL()
	{
	}

	_SelectableOctGridPCL::~_SelectableOctGridPCL()
	{
		stop();
	}

	bool _SelectableOctGridPCL::loadConfig(void)
	{
		IF_F(!this->_ModuleBase::loadConfig());
		const json &j = *m_pJ;

		IF_Le_F(j.contains("nMaxLines"), "Use nMaxCells for grid publication limits");
		jKv(j, "nMaxCells", m_nMaxCells);
		m_bColCellOcc = jKv<float>(j, "vColCellOcc", m_vColCellOcc);

		IF_Le_F(m_nMaxCells < 0, "Invalid grid cell limit");

		jKv(j, "nPminBuild", m_nPminBuild);
		IF_Le_F(m_nPminBuild < 0, "Invalid grid point threshold");
		m_cells.m_vCell.clear();
		m_buildCells.clear();
		m_cells.m_header = {};
		m_pOGpcl = nullptr;

		return true;
	}

	bool _SelectableOctGridPCL::link(InstanceMgr *pM)
	{
		IF_F(!_ModuleBase::link(pM));
		string name;
		jKv(*m_pJ, "_OctreeGridPCL", name);
		m_pOGpcl = dynamic_cast<_OctreeGridPCL *>(static_cast<BASE *>(pM->findModule(name)));
		IF_Le_F(!m_pOGpcl, "_OctreeGridPCL not found: " + name);
		std::lock_guard<std::mutex> gridLock(m_pOGpcl->m_gridMutex);
		std::lock_guard<std::mutex> cellsLock(m_cellsMutex);
		m_cells.m_header = gridHeader();
		m_cells.m_vCell.clear();
		return true;
	}

	bool _SelectableOctGridPCL::check(void)
	{
		return _ModuleBase::check() && m_pOGpcl && m_pOGpcl->check();
	}

	bool _SelectableOctGridPCL::start(void)
	{
		IF_F(!check());
		return m_pT->startThread(getUpdate, this);
	}

	void _SelectableOctGridPCL::stop(void)
	{
		if (m_pT) m_pT->join();
	}

	void _SelectableOctGridPCL::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();
			if (!m_pT->bRun()) break;

			std::lock_guard<std::mutex> gridLock(m_pOGpcl->m_gridMutex);
			updateDrawAssets();
		}
	}

	bool _SelectableOctGridPCL::saveConfig(bool bExport)
	{
		NULL_F(m_pOGpcl);
		std::lock_guard<std::mutex> gridLock(m_pOGpcl->m_gridMutex);
		return saveConfigLocked(bExport);
	}

	bool _SelectableOctGridPCL::saveConfigLocked(bool bExport)
	{
		IF_F(!_ModuleBase::saveConfig(false));
		IF_F(!m_pOGpcl->saveConfigLocked(false));

		json &j = *m_pJ;
		j["_OctreeGridPCL"] = m_pOGpcl->getName();
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
		if (m_pOGpcl->m_pJcfg != m_pJcfg && !m_pOGpcl->m_pJcfg->saveToFile())
			return false;
		return m_pJcfg->saveToFile();
	}

	json _SelectableOctGridPCL::gridRootJSON(void) const
	{
		json j = json::object();
		j["vPorigin"] = {json(m_pOGpcl->m_vPorigin.x()).dump(), json(m_pOGpcl->m_vPorigin.y()).dump(), json(m_pOGpcl->m_vPorigin.z()).dump()};
		j["vRootCellSize"] = {json(m_pOGpcl->m_vRootCellSize.x()).dump(), json(m_pOGpcl->m_vRootCellSize.y()).dump(), json(m_pOGpcl->m_vRootCellSize.z()).dump()};
		return j;
	}

	json _SelectableOctGridPCL::gridConfigJSON(void) const
	{
		json j = gridRootJSON();
		j.update({{"dTexpireCell", std::to_string(m_pOGpcl->m_dTexpireCell)}, {"dTexpirePCL", std::to_string(m_pOGpcl->m_dTexpirePCL)},
				  {"nMaxCells", std::to_string(m_nMaxCells)}, {"nMaxLevel", std::to_string(m_pOGpcl->m_nMaxLevel)},
				  {"nPminBuild", std::to_string(m_nPminBuild)}});
		return j;
	}

	json _SelectableOctGridPCL::cellSelectionJSON(void)
	{
		json j = gridRootJSON();
		j["nMaxLevel"] = m_pOGpcl->m_nMaxLevel;
		j["cellIDs"] = m_pOGpcl->selectedCellIDsJSON();
		j["tStamp"] = std::to_string(getTns());
		return j;
	}

	SELECTABLE_OCTGRID_HEADER _SelectableOctGridPCL::gridHeader(void) const
	{
		const auto &origin = m_pOGpcl->m_vPorigin;
		const auto &size = m_pOGpcl->m_vRootCellSize;
		return {{origin.x(), origin.y(), origin.z()}, {size.x(), size.y(), size.z()},
			uint32_t(m_pOGpcl->m_nMaxLevel), getTns()};
	}

	void _SelectableOctGridPCL::updateDrawAssets(void)
	{
		IF_(!check());
		NULL_(m_pOGpcl->m_pCell);

		// Traverse outside the publication lock so viewers only wait for a swap.
		m_buildCells.clear();
		addCellsRecursive(m_pOGpcl->m_pCell, m_nPminBuild, m_buildCells, m_nMaxCells, m_bColCellOcc ? &m_vColCellOcc : nullptr);
		std::lock_guard<std::mutex> lock(m_cellsMutex);
		m_cells.m_header = gridHeader();
		m_cells.m_vCell.swap(m_buildCells);
	}

	int _SelectableOctGridPCL::get(SELECTABLE_OCTGRID_CELLS *pOut, uint64_t tExpire, size_t nMaxCells)
	{
		NULL__(pOut, 0);
		std::lock_guard<std::mutex> lock(m_cellsMutex);
		pOut->m_header = m_cells.m_header;
		const size_t n = bExpired(m_cells.m_header.m_tStamp, tExpire) ? 0 : std::min(nMaxCells, m_cells.m_vCell.size());
		pOut->m_vCell.assign(m_cells.m_vCell.begin(), m_cells.m_vCell.begin() + n);
		return int(n);
	}

	bool _SelectableOctGridPCL::applyGridConfig(const json &j, string &error)
	{
		Vector3f origin = Vector3f::Zero(), size = Vector3f::Zero();
		uint64_t expireCell = 0, expirePCL = 0;
		int maxCells = 0, maxLevel = 0, minPoints = 0;
		const bool valid = readSelectionVector(jK(j, "vPorigin"), origin) &&
			readSelectionVector(jK(j, "vRootCellSize"), size) && size.x() > 0 && size.y() > 0 && size.z() > 0 &&
			readConfigInteger(jK(j, "dTexpireCell"), expireCell) && readConfigInteger(jK(j, "dTexpirePCL"), expirePCL) &&
			readConfigInteger(jK(j, "nMaxCells"), maxCells) && readConfigInteger(jK(j, "nMaxLevel"), maxLevel) &&
			maxLevel <= OCTGRID_MAX_LEVEL && readConfigInteger(jK(j, "nPminBuild"), minPoints);
		if (!valid)
		{
			error = "Provide all grid configuration fields as decimal strings: finite origin, positive root size, "
				"uint64 expiry durations, nonnegative int32 counts and nMaxLevel from 0 to 40";
			return false;
		}

		const bool geometryChanged = origin != m_pOGpcl->m_vPorigin || size != m_pOGpcl->m_vRootCellSize || maxLevel != m_pOGpcl->m_nMaxLevel;
		if (geometryChanged)
		{
			std::lock_guard<std::mutex> lock(m_cellsMutex);
			const SELECTABLE_OCTGRID_HEADER header = {{origin.x(), origin.y(), origin.z()}, {size.x(), size.y(), size.z()},
				uint32_t(maxLevel), getTns()};
			m_pOGpcl->m_vSelectedCells = remapSelectedCells(gridHeader(), header, m_pOGpcl->m_vSelectedCells);
			if (m_pOGpcl->m_pCell) m_pOGpcl->m_pCell->release();
			m_pOGpcl->resetPointInputs();
			m_buildCells.clear();
			m_cells.m_vCell.clear();
			m_pOGpcl->m_vPorigin = origin;
			m_pOGpcl->m_vRootCellSize = size;
			m_pOGpcl->m_nMaxLevel = maxLevel;
			m_cells.m_header = header; // Publish an empty snapshot until the next rebuild.
		}
		m_pOGpcl->m_dTexpireCell = expireCell;
		m_pOGpcl->m_dTexpirePCL = expirePCL;
		m_nMaxCells = maxCells;
		m_nPminBuild = minPoints;
		// Re-publish count/limit/expiry changes immediately, even without new input.
		if (!geometryChanged)
		{
			m_pOGpcl->deleteExpiredCells();
			updateDrawAssets();
		}
		return true;
	}

	bool _SelectableOctGridPCL::applyCellSelection(const json &j, string &error)
	{
		Vector3f origin = Vector3f::Zero(), size = Vector3f::Zero();
		const json *level = jK(j, "nMaxLevel"), *ids = jK(j, "cellIDs");
		if (!readSelectionVector(jK(j, "vPorigin"), origin) || !readSelectionVector(jK(j, "vRootCellSize"), size) ||
			origin != m_pOGpcl->m_vPorigin || size != m_pOGpcl->m_vRootCellSize || !level || !level->is_number_integer() || *level != m_pOGpcl->m_nMaxLevel)
		{
			error = "Selection root and numeric nMaxLevel must match the current grid";
			return false;
		}
		if (!ids || !ids->is_array())
		{
			error = "Provide cellIDs as an array of 32-character hexadecimal IDs";
			return false;
		}

		vector<UUID128> selected;
		selected.reserve(ids->size());
		for (const auto &value : *ids)
		{
			UUID128 id;
			if (!m_pOGpcl->readSelectionID(value, id) || !octgridValidCellID(id, m_pOGpcl->m_nMaxLevel))
			{
				error = "Invalid selected cell ID for the current grid";
				return false;
			}
			selected.push_back(id);
		}
		m_pOGpcl->m_vSelectedCells.swap(selected);
		return true;
	}

	void _SelectableOctGridPCL::jsonCmd(const json &j, void *pJSONbase)
	{
		string cmd;
		IF_(!jKv(j, "cmd", cmd));
		json reply = {{"cmd", cmd}, {"module", getName()}, {"bSuccess", false}};
		if (const json *requestId = jK(j, "requestId")) reply["requestId"] = *requestId;
		string error;
		if (!m_pOGpcl) error = "No linked _OctreeGridPCL";
		else
		{
			// Serialize Apply, Load and Save against updates and one another.
			std::lock_guard<std::mutex> gridLock(m_pOGpcl->m_gridMutex);
			if (cmd == "getGridConfig" || cmd == "setGridConfig")
			{
				if (cmd == "getGridConfig" || applyGridConfig(j, error)) reply.update(gridConfigJSON());
			}
			else if (cmd == "getCellSelection" || cmd == "setCellSelection")
			{
				if (cmd == "getCellSelection" || applyCellSelection(j, error)) reply.update(cellSelectionJSON());
			}
			else if (cmd == "saveConfig")
			{
				if (!saveConfigLocked(true)) error = "Saving the grid configuration file failed";
			}
			else error = "Unsupported selectable grid command";
		}
		reply["bSuccess"] = error.empty();
		if (!error.empty()) reply["error"] = error;
		if (pJSONbase) static_cast<_JSONbase *>(pJSONbase)->sendJson(reply);
	}
}

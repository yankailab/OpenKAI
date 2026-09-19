#include "SelectableOctGridSources.h"
#include "../../Module/ModuleMgr.h"
#include <algorithm>
#include <cmath>
#include <set>

namespace kai
{
	bool SelectableOctGridSources::link(const json &j, ModuleMgr *manager, int nP, int nL, int nC, string &error)
	{
		auto fail = [&](const string &message) { error = message; return false; };
		if (!manager || nP < 0 || nL < 0 || nC < 0)
			return fail("Invalid viewer source manager or limits");
		for (const char *key : {"vReferenceFrame", "vGeometryBase", "geometry"})
			if (j.contains(key))
				return fail(string("Unsupported viewer key: ") + key + "; use vGeometry and vSelectableOctGrid");

		SelectableOctGridSources sources;
		std::set<string> names;
		for (const char *key : {"vGeometry", "vSelectableOctGrid"})
		{
			if (!j.contains(key)) continue;
			const auto &entries = j.at(key);
			if (!entries.is_array()) return fail(string(key) + " must be an array");
			const bool grid = string(key) == "vSelectableOctGrid";
			const char *sourceKey = grid ? "_SelectableOctGrid" : "_GeometryBase";
			const std::set<string> allowed = grid
				? std::set<string>{sourceKey, "nC", "bVisible", "matCol", "matLineWidth"}
				: std::set<string>{sourceKey, "nP", "nL", "bVisible", "matCol", "matPointSize", "matLineWidth"};
			for (const auto &entry : entries)
			{
				if (!entry.is_object()) return fail(string(key) + " entries must be objects");
				for (auto it = entry.begin(); it != entry.end(); ++it)
					if (!allowed.count(it.key())) return fail("Unsupported source setting: " + it.key());
				VIEWER_SOURCE_STYLE style;
				if (!jKv(entry, sourceKey, style.m_name) || style.m_name.empty())
					return fail(string(key) + " entry needs " + sourceKey);
				if (!names.insert(style.m_name).second) return fail("Duplicate viewer source: " + style.m_name);
				jKv(entry, "bVisible", style.m_bVisible);
				jKv(entry, "matLineWidth", style.m_matLineWidth);
				jKv<float>(entry, "matCol", style.m_matCol);
				if (!style.m_matCol.allFinite() || !std::isfinite(style.m_matLineWidth) || style.m_matLineWidth <= 0)
					return fail("Invalid viewer material: " + style.m_name);

				auto *module = static_cast<BASE *>(manager->findModule(style.m_name));
				if (!module)
				{
					// Disabled modules can remain in a shared application's viewer config.
					const auto &config = manager->findJson(style.m_name);
					int enabled = 1;
					jKv(config, "bON", enabled);
					if (config.is_object() && !enabled) continue;
					return fail("Viewer source not found: " + style.m_name);
				}
				if (grid)
				{
					VIEWER_GRID_SOURCE source;
					static_cast<VIEWER_SOURCE_STYLE &>(source) = style;
					source.m_pGrid = dynamic_cast<_SelectableOctGrid *>(module);
					if (!source.m_pGrid) return fail("Not a selectable grid: " + style.m_name);
					source.m_nC = nC;
					jKv(entry, "nC", source.m_nC);
					if (source.m_nC < 0) return fail("Negative cell limit: " + style.m_name);
					source.m_nC = std::min(source.m_nC, nC);
					sources.m_vGrid.push_back(source);
				}
				else
				{
					VIEWER_GEOMETRY_SOURCE source;
					static_cast<VIEWER_SOURCE_STYLE &>(source) = style;
					source.m_pGeometry = dynamic_cast<_GeometryBase *>(module);
					if (!source.m_pGeometry) return fail("Not a geometry source: " + style.m_name);
					source.m_nP = nP; source.m_nL = nL;
					jKv(entry, "nP", source.m_nP);
					jKv(entry, "nL", source.m_nL);
					jKv(entry, "matPointSize", source.m_matPointSize);
					if (source.m_nP < 0 || source.m_nL < 0 || !std::isfinite(source.m_matPointSize) || source.m_matPointSize <= 0)
						return fail("Invalid geometry limits/material: " + style.m_name);
					source.m_nP = std::min(source.m_nP, nP);
					source.m_nL = std::min(source.m_nL, nL);
					sources.m_vGeometry.push_back(source);
				}
			}
		}
		*this = std::move(sources);
		return true;
	}
}

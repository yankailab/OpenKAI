#include "SelectableOctGridSources.h"
#include "../../Instance/InstanceMgr.h"
#include <algorithm>
#include <cmath>
#include <set>

namespace kai
{
	namespace
	{
		bool sourceError(string &error, const string &message)
		{
			error = message;
			return false;
		}
	}

	bool SelectableOctGridSources::link(const json &j, InstanceMgr *manager, int nP, int nL, int nC, string &error)
	{
		if (!manager || nP < 0 || nL < 0 || nC < 0)
		{
			return sourceError(error, "Invalid viewer source manager or limits");
		}

		for (const char *key : {"vReferenceFrame", "vGeometryBase", "geometry"})
		{
			if (j.contains(key))
			{
				return sourceError(error, string("Unsupported viewer key: ") + key + "; use vGeometry DataObjects and vSelectableOctGrid");
			}
		}

		SelectableOctGridSources sources;
		std::set<string> names;
		for (const char *key : {"vGeometry", "vSelectableOctGrid"})
		{
			if (!j.contains(key))
			{
				continue;
			}
			const auto &entries = j.at(key);
			if (!entries.is_array())
			{
				return sourceError(error, string(key) + " must be an array");
			}
			const bool grid = string(key) == "vSelectableOctGrid";
			const std::set<string> allowed = grid
				? std::set<string>{"_SelectableOctGrid", "nC", "bVisible", "matCol", "matLineWidth"}
				: std::set<string>{"PCLframeIn", "LineFrameIn", "name", "nP", "nL", "bVisible", "matCol", "matPointSize", "matLineWidth"};
			for (const auto &entry : entries)
			{
				if (!entry.is_object())
				{
					return sourceError(error, string(key) + " entries must be objects");
				}
				for (auto it = entry.begin(); it != entry.end(); ++it)
				{
					if (!allowed.count(it.key()))
					{
						return sourceError(error, "Unsupported source setting: " + it.key());
					}
				}
				VIEWER_SOURCE_STYLE style;
				string pointName;
				string lineName;
				if (grid)
				{
					jKv(entry, "_SelectableOctGrid", style.m_name);
				}
				else
				{
					jKv(entry, "PCLframeIn", pointName);
					jKv(entry, "LineFrameIn", lineName);
					if (pointName.empty() && lineName.empty())
					{
						return sourceError(error, "vGeometry entry needs PCLframeIn or LineFrameIn");
					}
					style.m_name = pointName.empty() ? lineName : pointName;
					jKv(entry, "name", style.m_name);
				}
				if (style.m_name.empty() || !names.insert(style.m_name).second)
				{
					return sourceError(error, "Empty or duplicate viewer source: " + style.m_name);
				}
				jKv(entry, "bVisible", style.m_bVisible);
				jKv(entry, "matLineWidth", style.m_matLineWidth);
				jKv<float>(entry, "matCol", style.m_matCol);
				if (!style.m_matCol.allFinite() || !std::isfinite(style.m_matLineWidth) || style.m_matLineWidth <= 0)
				{
					return sourceError(error, "Invalid viewer material: " + style.m_name);
				}

				if (grid)
				{
					VIEWER_GRID_SOURCE source;
					static_cast<VIEWER_SOURCE_STYLE &>(source) = style;
					auto *module = static_cast<BASE *>(manager->findModule(style.m_name));
					source.m_pGrid = dynamic_cast<_SelectableOctGrid *>(module);
					if (!source.m_pGrid)
					{
						const json *config = manager->findJson(style.m_name);
						bool enabled = true;
						if (config)
						{
							jKv(*config, "bON", enabled);
						}
						if (!module && config && !enabled)
						{
							continue;
						}
						return sourceError(error, "Selectable grid not found: " + style.m_name);
					}
					source.m_nC = nC;
					jKv(entry, "nC", source.m_nC);
					if (source.m_nC < 0)
					{
						return sourceError(error, "Negative cell limit: " + style.m_name);
					}
					source.m_nC = std::min(source.m_nC, nC);
					sources.m_vGrid.push_back(source);
					continue;
				}

				VIEWER_GEOMETRY_SOURCE source;
				static_cast<VIEWER_SOURCE_STYLE &>(source) = style;
				if (!pointName.empty())
				{
					source.m_pPCLframein = dynamic_cast<PCLframe *>(static_cast<DataObjBase *>(manager->findDataObject(pointName)));
					if (!source.m_pPCLframein)
					{
						return sourceError(error, "PCLframeIn not found: " + pointName);
					}
				}
				if (!lineName.empty())
				{
					source.m_pLineFramein = dynamic_cast<LineFrame *>(static_cast<DataObjBase *>(manager->findDataObject(lineName)));
					if (!source.m_pLineFramein)
					{
						return sourceError(error, "LineFrameIn not found: " + lineName);
					}
				}
				source.m_nP = nP;
				source.m_nL = nL;
				jKv(entry, "nP", source.m_nP);
				jKv(entry, "nL", source.m_nL);
				jKv(entry, "matPointSize", source.m_matPointSize);
				if (source.m_nP < 0 || source.m_nL < 0 || !std::isfinite(source.m_matPointSize) || source.m_matPointSize <= 0)
				{
					return sourceError(error, "Invalid geometry limits/material: " + style.m_name);
				}
				source.m_nP = source.m_pPCLframein ? std::min(source.m_nP, nP) : 0;
				source.m_nL = source.m_pLineFramein ? std::min(source.m_nL, nL) : 0;
				sources.m_vGeometry.push_back(source);
			}
		}
		*this = std::move(sources);
		return true;
	}
}

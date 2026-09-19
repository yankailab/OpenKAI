#ifndef OpenKAI_src_UI_Viewer_SelectableOctGridSources_H_
#define OpenKAI_src_UI_Viewer_SelectableOctGridSources_H_

#include "../../Universe/Geometry/_GeometryBase.h"
#include "../../Universe/Grid/_SelectableOctGrid.h"

namespace kai
{
	struct VIEWER_SOURCE_STYLE
	{
		string m_name;
		bool m_bVisible = true;
		Vector4f m_matCol = {1, 1, 1, 1};
		float m_matLineWidth = 1;
	};

	struct VIEWER_GEOMETRY_SOURCE : VIEWER_SOURCE_STYLE
	{
		_GeometryBase *m_pGeometry = nullptr;
		int m_nP = 0, m_nL = 0;
		float m_matPointSize = 2;
	};

	struct VIEWER_GRID_SOURCE : VIEWER_SOURCE_STYLE
	{
		_SelectableOctGrid *m_pGrid = nullptr;
		int m_nC = 0;
	};

	// Sources are resolved once at link time. Collection uses only typed pointers.
	struct SelectableOctGridSources
	{
		vector<VIEWER_GEOMETRY_SOURCE> m_vGeometry;
		vector<VIEWER_GRID_SOURCE> m_vGrid;

		bool link(const json &j, ModuleMgr *manager, int nP, int nL, int nC, string &error);
	};
}
#endif

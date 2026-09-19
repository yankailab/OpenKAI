/*
 * _ImGUIselectableOctGrid.h
 *
 *  Created on: Jun 4, 2026
 *      Author: Codex
 */

#ifndef OpenKAI_src_UI_Viewer_ImGUI__ImGUIselectableOctGrid_H_
#define OpenKAI_src_UI_Viewer_ImGUI__ImGUIselectableOctGrid_H_

#include "../../../Universe/Geometry/_GeometryViewerBase.h"
#include "ImGUIbackend.h"
#include "../SelectableOctGridSources.h"

struct ImDrawList;
struct ImDrawCmd;

namespace kai
{
	class ImGUIglRenderer;

	struct IMGUI_VIEWER_POINT
	{
		Vector3f m_vP = Vector3f::Zero();
		Vector3f m_vC{0, 0, 0};
	};

	struct IMGUI_VIEWER_LINE
	{
		Vector3f m_vA = Vector3f::Zero();
		Vector3f m_vB = Vector3f::Zero();
		Vector3f m_vC{0, 0, 0};
	};

	struct IMGUI_VIEWER_BOX
	{
		UUID128 m_ID = 0;
		Vector3f m_vCenter = Vector3f::Zero(), m_vSize = Vector3f::Zero();
		Vector4f m_vC{0, 0, 0, 1};

		template <typename F> void forEachEdge(F draw) const
		{
			Vector3f vertices[8];
			for (unsigned i = 0; i < 8; ++i)
				vertices[i] = Vector3f(m_vCenter.x() + (i & 4 ? 0.5f : -0.5f) * m_vSize.x(),
					m_vCenter.y() + (i & 2 ? 0.5f : -0.5f) * m_vSize.y(),
					m_vCenter.z() + (i & 1 ? 0.5f : -0.5f) * m_vSize.z());
			for (unsigned i = 0; i < 8; ++i)
				for (unsigned bit : {1u, 2u, 4u})
					if (!(i & bit)) draw(vertices[i], vertices[i | bit]);
		}
	};

	// Render snapshot: values only, no module pointers or source configuration.
	struct IMGUI_VIEWER_OBJ
	{
		string m_name;

		float m_matPointSize = 2.0;
		float m_matLineWidth = 1.0;
		Vector4f m_matCol = {1, 1, 1, 1};

		vector<IMGUI_VIEWER_POINT> m_vP;
		vector<IMGUI_VIEWER_LINE> m_vL;
		OCTGRID_HEADER m_gridHeader;
		vector<IMGUI_VIEWER_BOX> m_vBox;

		void clearGeometry(void);
	};

	class _ImGUIselectableOctGrid : public _GeometryViewerBase
	{
	public:
		_ImGUIselectableOctGrid();
		virtual ~_ImGUIselectableOctGrid();

		virtual bool init(const json &j) override;
		virtual bool link(const json &j, ModuleMgr *pM) override;
		virtual bool start(void) override;
		virtual bool check(void) override;

		virtual void resetCamPose(void) override;
		virtual void setCamPose(const GVIEWER_CAM_POSE &camPose) override;
		virtual GVIEWER_CAM_POSE getCamPose(void) override;
		virtual void setCamProj(const GVIEWER_CAM_PROJ &camProj) override;
		virtual GVIEWER_CAM_PROJ getCamProj(void) override;

	protected:
		virtual void update(void) override;
		static void *getUpdate(void *This)
		{
			((_ImGUIselectableOctGrid *)This)->update();
			return nullptr;
		}

		virtual void updateAllGeometries(void) override;
		virtual void updateCamProj(void) override;
		virtual void updateCamPose(void) override;

		virtual void updateUI(void);
		static void *getUpdateUI(void *This)
		{
			((_ImGUIselectableOctGrid *)This)->updateUI();
			return nullptr;
		}

		void drawUI(void);
		void drawScene(const Vector2f &vCanvasPos, const Vector2f &vCanvasSize);
		void drawStatusPanel(void);
		void drawGrid(const Vector2f &vCanvasPos, const Vector2f &vCanvasSize);
		void updateCameraControl(const Vector2f &vCanvasSize);
		void drawSceneCPU(const Vector2f &vCanvasPos, const Vector2f &vCanvasSize);
		void drawSceneGL(const Vector2f &vCanvasPos, const Vector2f &vCanvasSize);
		void renderSceneGL(const Vector2f &vCanvasPos, const Vector2f &vCanvasSize);
		static void drawSceneGLCallback(const ImDrawList *pParentList, const ImDrawCmd *pCmd);

		void collectGeometry(const VIEWER_GEOMETRY_SOURCE &source, IMGUI_VIEWER_OBJ *pObj, uint64_t expiry);
		void collectPoints(const VIEWER_GEOMETRY_SOURCE &source, IMGUI_VIEWER_OBJ *pObj, uint64_t expiry);
		void collectLines(const VIEWER_GEOMETRY_SOURCE &source, IMGUI_VIEWER_OBJ *pObj, uint64_t expiry);
		void collectCells(const VIEWER_GRID_SOURCE &source, IMGUI_VIEWER_OBJ *pObj, uint64_t expiry);
		void copySnapshot(vector<IMGUI_VIEWER_OBJ> *pVgo);

		bool projectPoint(const Vector3f &vP,
						  const Vector2f &vCanvasPos,
						  const Vector2f &vCanvasSize,
						  Vector2f *pVscreen,
						  float *pDepth);
		void getCameraBasis(Vector3f *pForward, Vector3f *pRight, Vector3f *pUp);
		void orbit(float dYaw, float dPitch);
		void pan(float dx, float dy, const Vector2f &vCanvasSize);
		void zoom(float d);
		bool camBound(void);

		void snapshotLock(void);
		void snapshotUnlock(void);

	protected:
		SelectableOctGridSources m_sources;
		vector<IMGUI_VIEWER_OBJ> m_vBuildGO;
		vector<IMGUI_VIEWER_OBJ> m_vDrawGO;
		OCTGRID_CELLS m_cells;
		int m_nCbuf = 100000;

		ImGUIbackend *m_pBackend = nullptr;
		ImGUIglRenderer *m_pGLRenderer = nullptr;
		_Thread *m_pTui = nullptr;
		pthread_mutex_t m_snapshotMutex;

		bool m_bShowPanel = true;
		bool m_bShowGrid = true;
		bool m_bAutoBound = true;
		float m_sMove = 0.01;
		float m_sOrbit = 0.008;
		float m_sZoom = 0.1;
		float m_pointScale = 1.0;
		float m_lineScale = 1.0;
		Vector4f m_vBgCol = Vector4f::Zero();

		bool m_bGpuRender = true;
		unsigned long long m_snapshotVersion = 0;
		size_t m_nDrawObjects = 0;
		size_t m_nDrawPoints = 0;
		size_t m_nDrawLines = 0;
		Vector2f m_vGLCanvasPos = Vector2f::Zero();
		Vector2f m_vGLCanvasSize = Vector2f::Zero();
	};
}

#endif

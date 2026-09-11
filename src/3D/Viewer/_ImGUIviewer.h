/*
 * _ImGUIviewer.h
 *
 *  Created on: Jun 4, 2026
 *      Author: Codex
 */

#ifndef OpenKAI_src_3D_Viewer__ImGUIviewer_H_
#define OpenKAI_src_3D_Viewer__ImGUIviewer_H_

#include "../_GeometryViewerBase.h"
#include "ImGUIviewerBackend.h"
#include "../Grid/OctreeGridCells.h"

struct ImDrawList;
struct ImDrawCmd;

namespace kai
{
	class ImGUIviewerGLRenderer;

	struct IMGUI_VIEWER_POINT
	{
		vFloat3 m_vP;
		vFloat4 m_vC{0, 0, 0, 1};
	};

	struct IMGUI_VIEWER_LINE
	{
		vFloat3 m_vA;
		vFloat3 m_vB;
		vFloat4 m_vC{0, 0, 0, 1};
	};

	struct IMGUI_VIEWER_BOX
	{
		UUID128 m_ID = 0;
		vFloat3 m_vCenter, m_vSize;
		vFloat4 m_vC{0, 0, 0, 1};

		template <typename F> void forEachEdge(F draw) const
		{
			vFloat3 vertices[8];
			for (unsigned i = 0; i < 8; ++i)
				vertices[i] = vFloat3(m_vCenter.x + (i & 4 ? 0.5f : -0.5f) * m_vSize.x,
					m_vCenter.y + (i & 2 ? 0.5f : -0.5f) * m_vSize.y,
					m_vCenter.z + (i & 1 ? 0.5f : -0.5f) * m_vSize.z);
			for (unsigned i = 0; i < 8; ++i)
				for (unsigned bit : {1u, 2u, 4u})
					if (!(i & bit)) draw(vertices[i], vertices[i | bit]);
		}
	};

	struct IMGUI_VIEWER_OBJ
	{
		_GeometryBase *m_pGB = nullptr;
		string m_name;

		bool m_bVisible = true;
		int m_nPbuf = 0;
		int m_nLbuf = 0;
		int m_nCbuf = -1;
		float m_matPointSize = 2.0;
		float m_matLineWidth = 1.0;
		vFloat4 m_matCol = {1, 1, 1, 1};

		vector<IMGUI_VIEWER_POINT> m_vP;
		vector<IMGUI_VIEWER_LINE> m_vL;
		OCTGRID_HEADER m_gridHeader;
		vector<IMGUI_VIEWER_BOX> m_vBox;

		void reserve(int nPbufDefault = 0, int nLbufDefault = 0);
		void clearGeometry(void);
	};

	class _ImGUIviewer : public _GeometryViewerBase
	{
	public:
		_ImGUIviewer();
		virtual ~_ImGUIviewer();

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
			((_ImGUIviewer *)This)->update();
			return nullptr;
		}

		virtual void updateAllGeometries(void) override;
		virtual void updateCamProj(void) override;
		virtual void updateCamPose(void) override;

		virtual void updateUI(void);
		static void *getUpdateUI(void *This)
		{
			((_ImGUIviewer *)This)->updateUI();
			return nullptr;
		}

		void drawUI(void);
		void drawScene(const vFloat2 &vCanvasPos, const vFloat2 &vCanvasSize);
		void drawStatusPanel(void);
		void drawGrid(const vFloat2 &vCanvasPos, const vFloat2 &vCanvasSize);
		void updateCameraControl(const vFloat2 &vCanvasSize);
		void drawSceneCPU(const vFloat2 &vCanvasPos, const vFloat2 &vCanvasSize);
		void drawSceneGL(const vFloat2 &vCanvasPos, const vFloat2 &vCanvasSize);
		void renderSceneGL(const vFloat2 &vCanvasPos, const vFloat2 &vCanvasSize);
		static void drawSceneGLCallback(const ImDrawList *pParentList, const ImDrawCmd *pCmd);

		void collectGeometry(_GeometryBase *pGb, IMGUI_VIEWER_OBJ *pObj);
		void collectPoints(IMGUI_VIEWER_OBJ *pObj);
		void collectLines(IMGUI_VIEWER_OBJ *pObj);
		void collectCells(IMGUI_VIEWER_OBJ *pObj);
		void copySnapshot(vector<IMGUI_VIEWER_OBJ> *pVgo);

		bool upsertGeometry(_GeometryBase *pGb, const string &name, const json *pJ = nullptr);
		void applyObjectConfig(IMGUI_VIEWER_OBJ *pObj, const json &j);
		IMGUI_VIEWER_OBJ *findObject(_GeometryBase *pGb, const string &name = "");
		const IMGUI_VIEWER_OBJ *findObject(_GeometryBase *pGb, const string &name = "") const;

		bool projectPoint(const vFloat3 &vP,
						  const vFloat2 &vCanvasPos,
						  const vFloat2 &vCanvasSize,
						  vFloat2 *pVscreen,
						  float *pDepth);
		void getCameraBasis(vFloat3 *pForward, vFloat3 *pRight, vFloat3 *pUp);
		void orbit(float dYaw, float dPitch);
		void pan(float dx, float dy, const vFloat2 &vCanvasSize);
		void zoom(float d);
		bool camBound(void);

		void snapshotLock(void);
		void snapshotUnlock(void);

	protected:
		vector<IMGUI_VIEWER_OBJ> m_vGO;
		vector<IMGUI_VIEWER_OBJ> m_vBuildGO;
		vector<IMGUI_VIEWER_OBJ> m_vDrawGO;
		OCTGRID_CELLS m_cells;
		int m_nCbuf = 100000;

		ImGUIviewerBackend *m_pBackend = nullptr;
		ImGUIviewerGLRenderer *m_pGLRenderer = nullptr;
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
		vFloat4 m_vBgCol;

		bool m_bGpuRender = true;
		unsigned long long m_snapshotVersion = 0;
		size_t m_nDrawObjects = 0;
		size_t m_nDrawPoints = 0;
		size_t m_nDrawLines = 0;
		vFloat2 m_vGLCanvasPos;
		vFloat2 m_vGLCanvasSize;
	};
}

#endif

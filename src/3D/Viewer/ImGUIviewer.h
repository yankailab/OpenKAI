/*
 * ImGUIviewer.h
 *
 *  Created on: Jun 4, 2026
 *      Author: Codex
 */

#ifndef OpenKAI_src_3D_Viewer_ImGUIviewer_H_
#define OpenKAI_src_3D_Viewer_ImGUIviewer_H_

#include "../PointCloud/_PCstream.h"
#include "../PointCloud/_PCgridBase.h"
#include "ImGuiViewerBackend.h"

namespace kai
{
	struct IMGUI_VIEWER_CAM_PROJ
	{
		float m_fov = 70.0;
		vFloat2 m_vNF = {0.05, 10000.0};
	};

	struct IMGUI_VIEWER_CAM
	{
		vFloat3 m_vLookAt = {0, 0, 0};
		vFloat3 m_vEye = {0, 0, 5};
		vFloat3 m_vUp = {0, 1, 0};
	};

	struct IMGUI_VIEWER_POINT
	{
		vFloat3 m_vP;
		vFloat3 m_vC;
	};

	struct IMGUI_VIEWER_LINE
	{
		vFloat3 m_vA;
		vFloat3 m_vB;
		vFloat3 m_vC;
	};

	struct IMGUI_VIEWER_OBJ
	{
		_GeometryBase *m_pGB = nullptr;
		string m_name;

		bool m_bStatic = true;
		int m_nPbuf = 0;
		int m_iGridLS = 0;
		int m_matPointSize = 2;
		int m_matLineWidth = 1;
		vFloat4 m_matCol = {1, 1, 1, 1};

		vector<IMGUI_VIEWER_POINT> m_vP;
		vector<IMGUI_VIEWER_LINE> m_vL;

		void init(void);
		void updateGeometry(void);
		void addPCstream(const uint64_t tExpire = 0);
		void addLineSet(const LineSet &ls);
	};

	class ImGUIviewer : public _GeometryBase
	{
	public:
		ImGUIviewer();
		virtual ~ImGUIviewer();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual bool check(void);

		virtual void resetCamPose(void);
		virtual void setCamPose(const IMGUI_VIEWER_CAM &camPose);
		virtual IMGUI_VIEWER_CAM getCamPose(void);

	protected:
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((ImGUIviewer *)This)->update();
			return nullptr;
		}

		virtual void updateUI(void);
		static void *getUpdateUI(void *This)
		{
			((ImGUIviewer *)This)->updateUI();
			return nullptr;
		}

		void drawUI(void);
		void drawScene(const vFloat2 &vCanvasPos, const vFloat2 &vCanvasSize);
		void drawStatusPanel(void);
		void updateCameraControl(const vFloat2 &vCanvasSize);
		void copySnapshot(vector<IMGUI_VIEWER_OBJ> *pVgo);

		bool projectPoint(const vFloat3 &vP,
						  const vFloat2 &vCanvasPos,
						  const vFloat2 &vCanvasSize,
						  vFloat2 *pVscreen,
						  float *pDepth);
		void getCameraBasis(vFloat3 *pForward, vFloat3 *pRight, vFloat3 *pUp);
		void orbit(float dYaw, float dPitch);
		void pan(float dx, float dy);
		void zoom(float d);
		bool camBound(void);

		void snapshotLock(void);
		void snapshotUnlock(void);

	protected:
		vector<IMGUI_VIEWER_OBJ> m_vGO;
		vector<IMGUI_VIEWER_OBJ> m_vDrawGO;

		ImGuiViewerBackend *m_pBackend;
		_Thread *m_pTui;
		pthread_mutex_t m_snapshotMutex;

		vInt2 m_vWinSize;
		bool m_bFullScreen;
		bool m_bShowPanel;
		bool m_bShowGrid;
		bool m_bAutoBound;
		float m_sMove;
		float m_sOrbit;
		float m_sZoom;
		float m_pointScale;
		float m_lineScale;
		vFloat4 m_vBgCol;

		IMGUI_VIEWER_CAM_PROJ m_camProj;
		IMGUI_VIEWER_CAM m_cam;
		IMGUI_VIEWER_CAM m_camDefault;
		vFloat3 m_vCoR;
	};
}

#endif

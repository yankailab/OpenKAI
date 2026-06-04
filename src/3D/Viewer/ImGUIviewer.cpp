/*
 * ImGUIviewer.cpp
 *
 *  Created on: Jun 4, 2026
 *      Author: Codex
 */

#include "ImGUIviewer.h"

#include "imgui.h"
#include <algorithm>
#include <cmath>

namespace kai
{
	static float vDot(const vFloat3 &a, const vFloat3 &b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	static vFloat3 vCross(const vFloat3 &a, const vFloat3 &b)
	{
		return vFloat3(a.y * b.z - a.z * b.y,
					   a.z * b.x - a.x * b.z,
					   a.x * b.y - a.y * b.x);
	}

	static vFloat3 vNorm(const vFloat3 &v)
	{
		float l = sqrt(vDot(v, v));
		if (l <= 1e-6)
			return vFloat3(0, 0, 0);

		return vFloat3(v.x / l, v.y / l, v.z / l);
	}

	static ImU32 colU32(const vFloat4 &c, float alphaScale = 1.0)
	{
		return IM_COL32((int)(std::clamp(c.x, 0.0f, 1.0f) * 255.0f),
						(int)(std::clamp(c.y, 0.0f, 1.0f) * 255.0f),
						(int)(std::clamp(c.z, 0.0f, 1.0f) * 255.0f),
						(int)(std::clamp(c.w * alphaScale, 0.0f, 1.0f) * 255.0f));
	}

	static ImU32 colU32(const vFloat3 &c, float alpha = 1.0)
	{
		return IM_COL32((int)(std::clamp(c.x, 0.0f, 1.0f) * 255.0f),
						(int)(std::clamp(c.y, 0.0f, 1.0f) * 255.0f),
						(int)(std::clamp(c.z, 0.0f, 1.0f) * 255.0f),
						(int)(std::clamp(alpha, 0.0f, 1.0f) * 255.0f));
	}

	void IMGUI_VIEWER_OBJ::init(void)
	{
		if (m_nPbuf > 0)
			m_vP.reserve(m_nPbuf);
	}

	void IMGUI_VIEWER_OBJ::updateGeometry(void)
	{
		if (!m_pGB)
			return;

		GEOMETRY_TYPE gt = m_pGB->getType();
		if (gt == pc_stream)
		{
			addPCstream();
		}
		else if (gt == pc_grid)
		{
			_PCgridBase *p = (_PCgridBase *)m_pGB;
			if (m_bStatic)
				addLineSet(*p->getGridLines());
			else
			{
				LineSet ls;
				p->getActiveCellLines(&ls, m_iGridLS);
				addLineSet(ls);
			}
		}
	}

	void IMGUI_VIEWER_OBJ::addPCstream(const uint64_t tExpire)
	{
		_PCstream *p = (_PCstream *)m_pGB;
		m_vP.clear();

		int nP = p->nP();
		if (m_nPbuf > 0)
			nP = std::min(nP, m_nPbuf);

		for (int i = 0; i < nP; i++)
		{
			GEOMETRY_POINT *pP = p->get(i);
			if (!pP)
				continue;
			if (tExpire > 0 && pP->m_tStamp < tExpire)
				continue;

			IMGUI_VIEWER_POINT vp;
			vp.m_vP = pP->m_vP;
			vp.m_vC = pP->m_vC;
			m_vP.push_back(vp);
		}
	}

	void IMGUI_VIEWER_OBJ::addLineSet(const LineSet &ls)
	{
		m_vL.clear();

		for (const Vector2i &l : ls.lines_)
		{
			if (l.x() < 0 || l.y() < 0)
				continue;
			if (l.x() >= (int)ls.points_.size() || l.y() >= (int)ls.points_.size())
				continue;

			IMGUI_VIEWER_LINE vl;
			Vector3d a = ls.points_[l.x()];
			Vector3d b = ls.points_[l.y()];
			vl.m_vA.set(a.x(), a.y(), a.z());
			vl.m_vB.set(b.x(), b.y(), b.z());
			vl.m_vC.set(m_matCol.x, m_matCol.y, m_matCol.z);
			if (ls.colors_.size() > 0)
			{
				Vector3d c = ls.colors_[std::min((int)ls.colors_.size() - 1, (int)m_vL.size())];
				vl.m_vC.set(c.x(), c.y(), c.z());
			}

			m_vL.push_back(vl);
		}
	}

	ImGUIviewer::ImGUIviewer()
	{
		m_pBackend = nullptr;
		m_pTui = nullptr;
		m_vWinSize.set(1280, 720);
		m_bFullScreen = false;
		m_bShowPanel = true;
		m_bShowGrid = true;
		m_bAutoBound = true;
		m_sMove = 0.01;
		m_sOrbit = 0.008;
		m_sZoom = 0.1;
		m_pointScale = 1.0;
		m_lineScale = 1.0;
		m_vBgCol.set(0.05, 0.055, 0.06, 1.0);
		m_vCoR.set(0, 0, 0);

		pthread_mutex_init(&m_snapshotMutex, NULL);
	}

	ImGUIviewer::~ImGUIviewer()
	{
		DEL(m_pBackend);
		pthread_mutex_destroy(&m_snapshotMutex);
	}

	bool ImGUIviewer::init(const json &j)
	{
		IF_F(!this->_GeometryBase::init(j));

		jKv<int>(j, "vWinSize", m_vWinSize);
		jKv(j, "bFullScreen", m_bFullScreen);
		jKv(j, "bShowPanel", m_bShowPanel);
		jKv(j, "bShowGrid", m_bShowGrid);
		jKv(j, "bAutoBound", m_bAutoBound);
		jKv(j, "sMove", m_sMove);
		jKv(j, "sOrbit", m_sOrbit);
		jKv(j, "sZoom", m_sZoom);
		jKv(j, "pointScale", m_pointScale);
		jKv(j, "lineScale", m_lineScale);
		jKv<float>(j, "vBgCol", m_vBgCol);

		jKv(j, "camFov", m_camProj.m_fov);
		jKv<float>(j, "vCamNF", m_camProj.m_vNF);
		jKv<float>(j, "vCamLookAt", m_camDefault.m_vLookAt);
		jKv<float>(j, "vCamEye", m_camDefault.m_vEye);
		jKv<float>(j, "vCamUp", m_camDefault.m_vUp);
		jKv<float>(j, "vCoR", m_vCoR);
		m_cam = m_camDefault;

		DEL(m_pTui);
		m_pTui = createThread(jK(j, "threadUI"), "threadUI");
		NULL_F(m_pTui);

		return true;
	}

	bool ImGUIviewer::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_GeometryBase::link(j, pM));

		const json &jg = jK(j, "geometry");
		IF__(!jg.is_array(), true);

		for (auto it = jg.begin(); it != jg.end(); it++)
		{
			const json &Ji = it.value();
			IF_CONT(!Ji.is_object());

			string n = "";
			jKv(Ji, "_GeometryBase", n);
			_GeometryBase *pGB = (_GeometryBase *)(pM->findModule(n));
			IF_CONT(!pGB);

			IMGUI_VIEWER_OBJ g;
			g.m_pGB = pGB;
			g.m_name = n;
			jKv(Ji, "bStatic", g.m_bStatic);
			jKv(Ji, "nP", g.m_nPbuf);
			jKv<float>(Ji, "matCol", g.m_matCol);
			jKv(Ji, "matPointSize", g.m_matPointSize);
			jKv(Ji, "matLineWidth", g.m_matLineWidth);
			jKv(Ji, "iGridLS", g.m_iGridLS);

			g.init();
			m_vGO.push_back(g);
		}

		return true;
	}

	bool ImGUIviewer::start(void)
	{
		NULL_F(m_pT);
		IF_F(!m_pT->startThread(getUpdate, this));

		NULL_F(m_pTui);
		IF_F(!m_pTui->startThread(getUpdateUI, this));

		return true;
	}

	bool ImGUIviewer::check(void)
	{
		return this->_GeometryBase::check();
	}

	void ImGUIviewer::update(void)
	{
		m_pT->sleepT(USEC_1SEC);

		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			for (IMGUI_VIEWER_OBJ &g : m_vGO)
			{
				if (!g.m_bStatic || g.m_vP.empty() || g.m_vL.empty())
					g.updateGeometry();
			}

			snapshotLock();
			m_vDrawGO = m_vGO;
			snapshotUnlock();
		}
	}

	void ImGUIviewer::updateUI(void)
	{
		m_pBackend = createImGuiViewerBackend();
		NULL_(m_pBackend);
		IF_(!m_pBackend->init(this->getName(), m_vWinSize.x, m_vWinSize.y, m_bFullScreen));

		m_pT->run();

		while (m_pTui->bAlive() && !m_pBackend->bClose())
		{
			m_pTui->autoFPS();
			m_pBackend->beginFrame();
			drawUI();

			float c[4] = {m_vBgCol.x, m_vBgCol.y, m_vBgCol.z, m_vBgCol.w};
			m_pBackend->endFrame(c);
		}

		m_pBackend->shutdown();
		exit(0);
	}

	void ImGUIviewer::drawUI(void)
	{
		ImGuiViewport *pViewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(pViewport->WorkPos);
		ImGui::SetNextWindowSize(pViewport->WorkSize);

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
								 ImGuiWindowFlags_NoMove |
								 ImGuiWindowFlags_NoSavedSettings |
								 ImGuiWindowFlags_NoBringToFrontOnFocus |
								 ImGuiWindowFlags_NoNavFocus;

		ImGui::Begin("OpenKAI ImGui 3D Viewer", nullptr, flags);

		ImVec2 canvasPos = ImGui::GetCursorScreenPos();
		ImVec2 canvasSize = ImGui::GetContentRegionAvail();
		if (canvasSize.x < 1.0)
			canvasSize.x = 1.0;
		if (canvasSize.y < 1.0)
			canvasSize.y = 1.0;

		ImGui::InvisibleButton("viewer_canvas", canvasSize,
							   ImGuiButtonFlags_MouseButtonLeft |
								   ImGuiButtonFlags_MouseButtonMiddle |
								   ImGuiButtonFlags_MouseButtonRight);
		updateCameraControl(vFloat2(canvasSize.x, canvasSize.y));
		drawScene(vFloat2(canvasPos.x, canvasPos.y), vFloat2(canvasSize.x, canvasSize.y));
		ImGui::End();

		if (m_bShowPanel)
			drawStatusPanel();
	}

	void ImGUIviewer::drawStatusPanel(void)
	{
		ImGui::SetNextWindowPos(ImVec2(12, 12), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(300, 0), ImGuiCond_Once);
		ImGui::Begin("Viewer", &m_bShowPanel, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Backend: %s", getImGuiViewerBackendName());
		vector<IMGUI_VIEWER_OBJ> vGO;
		copySnapshot(&vGO);
		ImGui::Text("Geometry: %zu", vGO.size());
		ImGui::Separator();
		if (ImGui::Button("Reset camera"))
			resetCamPose();
		ImGui::SameLine();
		if (ImGui::Button("Auto bound"))
			camBound();
		ImGui::Checkbox("Grid", &m_bShowGrid);
		ImGui::SliderFloat("Point scale", &m_pointScale, 0.25, 8.0);
		ImGui::SliderFloat("Line scale", &m_lineScale, 0.25, 8.0);
		ImGui::ColorEdit4("Background", &m_vBgCol.x);
		ImGui::End();
	}

	void ImGUIviewer::drawScene(const vFloat2 &vCanvasPos, const vFloat2 &vCanvasSize)
	{
		ImDrawList *pDraw = ImGui::GetWindowDrawList();
		ImVec2 p0(vCanvasPos.x, vCanvasPos.y);
		ImVec2 p1(vCanvasPos.x + vCanvasSize.x, vCanvasPos.y + vCanvasSize.y);
		pDraw->AddRectFilled(p0, p1, colU32(m_vBgCol));

		vector<IMGUI_VIEWER_OBJ> vGO;
		copySnapshot(&vGO);

		if (m_bAutoBound && camBound())
			m_bAutoBound = false;

		if (m_bShowGrid)
		{
			const float r = 10.0;
			const ImU32 c = IM_COL32(70, 75, 82, 180);
			for (int i = -10; i <= 10; i++)
			{
				vFloat2 a, b;
				float dA = 0;
				float dB = 0;
				if (projectPoint(vFloat3(i, -r, 0), vCanvasPos, vCanvasSize, &a, &dA) &&
					projectPoint(vFloat3(i, r, 0), vCanvasPos, vCanvasSize, &b, &dB))
					pDraw->AddLine(ImVec2(a.x, a.y), ImVec2(b.x, b.y), c, 1.0);
				if (projectPoint(vFloat3(-r, i, 0), vCanvasPos, vCanvasSize, &a, &dA) &&
					projectPoint(vFloat3(r, i, 0), vCanvasPos, vCanvasSize, &b, &dB))
					pDraw->AddLine(ImVec2(a.x, a.y), ImVec2(b.x, b.y), c, 1.0);
			}
		}

		for (const IMGUI_VIEWER_OBJ &g : vGO)
		{
			for (const IMGUI_VIEWER_LINE &l : g.m_vL)
			{
				vFloat2 a, b;
				float dA = 0;
				float dB = 0;
				if (!projectPoint(l.m_vA, vCanvasPos, vCanvasSize, &a, &dA))
					continue;
				if (!projectPoint(l.m_vB, vCanvasPos, vCanvasSize, &b, &dB))
					continue;

				pDraw->AddLine(ImVec2(a.x, a.y), ImVec2(b.x, b.y),
							   colU32(l.m_vC, g.m_matCol.w),
							   std::max(1.0f, (float)g.m_matLineWidth * m_lineScale));
			}

			for (const IMGUI_VIEWER_POINT &p : g.m_vP)
			{
				vFloat2 vS;
				float d = 0;
				if (!projectPoint(p.m_vP, vCanvasPos, vCanvasSize, &vS, &d))
					continue;

				float r = std::max(1.0f, (float)g.m_matPointSize * m_pointScale);
				pDraw->AddCircleFilled(ImVec2(vS.x, vS.y), r, colU32(p.m_vC, g.m_matCol.w), 8);
			}
		}
	}

	void ImGUIviewer::updateCameraControl(const vFloat2 &vCanvasSize)
	{
		if (!ImGui::IsItemHovered())
			return;

		ImGuiIO &io = ImGui::GetIO();
		if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			orbit(io.MouseDelta.x * m_sOrbit, io.MouseDelta.y * m_sOrbit);
		if (ImGui::IsMouseDragging(ImGuiMouseButton_Right) || ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
			pan(io.MouseDelta.x / std::max(1.0f, vCanvasSize.x), io.MouseDelta.y / std::max(1.0f, vCanvasSize.y));
		if (fabs(io.MouseWheel) > 1e-6)
			zoom(io.MouseWheel);
	}

	void ImGUIviewer::copySnapshot(vector<IMGUI_VIEWER_OBJ> *pVgo)
	{
		NULL_(pVgo);

		snapshotLock();
		*pVgo = m_vDrawGO;
		snapshotUnlock();
	}

	bool ImGUIviewer::projectPoint(const vFloat3 &vP,
								   const vFloat2 &vCanvasPos,
								   const vFloat2 &vCanvasSize,
								   vFloat2 *pVscreen,
								   float *pDepth)
	{
		NULL_F(pVscreen);

		vFloat3 f, r, u;
		getCameraBasis(&f, &r, &u);

		vFloat3 d = {vP.x - m_cam.m_vEye.x, vP.y - m_cam.m_vEye.y, vP.z - m_cam.m_vEye.z};
		float x = vDot(d, r);
		float y = vDot(d, u);
		float z = vDot(d, f);
		IF_F(z < m_camProj.m_vNF.x);
		IF_F(z > m_camProj.m_vNF.y);

		float fov = std::clamp(m_camProj.m_fov, 10.0f, 140.0f) * OK_PI / 180.0;
		float sy = 1.0 / tan(fov * 0.5);
		float sx = sy * vCanvasSize.y / std::max(1.0f, vCanvasSize.x);

		float nx = (x * sx) / z;
		float ny = (y * sy) / z;
		IF_F(nx < -1.5 || nx > 1.5 || ny < -1.5 || ny > 1.5);

		pVscreen->x = vCanvasPos.x + (nx * 0.5 + 0.5) * vCanvasSize.x;
		pVscreen->y = vCanvasPos.y + (-ny * 0.5 + 0.5) * vCanvasSize.y;

		if (pDepth)
			*pDepth = z;

		return true;
	}

	void ImGUIviewer::getCameraBasis(vFloat3 *pForward, vFloat3 *pRight, vFloat3 *pUp)
	{
		vFloat3 f = vNorm(m_cam.m_vLookAt - m_cam.m_vEye);
		if (f.len() <= 1e-6)
			f.set(0, 0, -1);

		vFloat3 r = vNorm(vCross(f, m_cam.m_vUp));
		if (r.len() <= 1e-6)
			r.set(1, 0, 0);

		vFloat3 u = vNorm(vCross(r, f));

		if (pForward)
			*pForward = f;
		if (pRight)
			*pRight = r;
		if (pUp)
			*pUp = u;
	}

	void ImGUIviewer::orbit(float dYaw, float dPitch)
	{
		vFloat3 v = m_cam.m_vEye - m_cam.m_vLookAt;
		float r = std::max(0.01f, v.len());
		float yaw = atan2(v.y, v.x) - dYaw;
		float pitch = asin(std::clamp(v.z / r, -0.99f, 0.99f)) + dPitch;
		pitch = std::clamp(pitch, -1.45f, 1.45f);

		float cp = cos(pitch);
		m_cam.m_vEye.x = m_cam.m_vLookAt.x + r * cp * cos(yaw);
		m_cam.m_vEye.y = m_cam.m_vLookAt.y + r * cp * sin(yaw);
		m_cam.m_vEye.z = m_cam.m_vLookAt.z + r * sin(pitch);
	}

	void ImGUIviewer::pan(float dx, float dy)
	{
		vFloat3 f, r, u;
		getCameraBasis(&f, &r, &u);
		float d = std::max(0.01f, (m_cam.m_vEye - m_cam.m_vLookAt).len());
		vFloat3 move = (r * (-dx * d * m_sMove * 100.0)) + (u * (dy * d * m_sMove * 100.0));
		m_cam.m_vEye += move;
		m_cam.m_vLookAt += move;
	}

	void ImGUIviewer::zoom(float d)
	{
		vFloat3 v = m_cam.m_vEye - m_cam.m_vLookAt;
		float s = std::max(0.05f, 1.0f - d * m_sZoom);
		m_cam.m_vEye = m_cam.m_vLookAt + (v * s);
	}

	bool ImGUIviewer::camBound(void)
	{
		vector<IMGUI_VIEWER_OBJ> vGO;
		copySnapshot(&vGO);

		bool bFound = false;
		vFloat3 vMin(FLT_MAX, FLT_MAX, FLT_MAX);
		vFloat3 vMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		auto expand = [&](const vFloat3 &p)
		{
			vMin.x = std::min(vMin.x, p.x);
			vMin.y = std::min(vMin.y, p.y);
			vMin.z = std::min(vMin.z, p.z);
			vMax.x = std::max(vMax.x, p.x);
			vMax.y = std::max(vMax.y, p.y);
			vMax.z = std::max(vMax.z, p.z);
			bFound = true;
		};

		for (const IMGUI_VIEWER_OBJ &g : vGO)
		{
			for (const IMGUI_VIEWER_POINT &p : g.m_vP)
				expand(p.m_vP);
			for (const IMGUI_VIEWER_LINE &l : g.m_vL)
			{
				expand(l.m_vA);
				expand(l.m_vB);
			}
		}

		if (!bFound)
			return false;

		vFloat3 c = (vMin + vMax) * 0.5;
		float radius = std::max((vMax - vMin).len() * 0.5f, 1.0f);
		vFloat3 f, r, u;
		getCameraBasis(&f, &r, &u);
		m_cam.m_vLookAt = c + m_vCoR;
		m_cam.m_vEye = m_cam.m_vLookAt - (f * (radius * 2.5));
		return true;
	}

	void ImGUIviewer::resetCamPose(void)
	{
		m_cam = m_camDefault;
	}

	void ImGUIviewer::setCamPose(const IMGUI_VIEWER_CAM &camPose)
	{
		m_cam = camPose;
	}

	IMGUI_VIEWER_CAM ImGUIviewer::getCamPose(void)
	{
		return m_cam;
	}

	void ImGUIviewer::snapshotLock(void)
	{
		pthread_mutex_lock(&m_snapshotMutex);
	}

	void ImGUIviewer::snapshotUnlock(void)
	{
		pthread_mutex_unlock(&m_snapshotMutex);
	}
}

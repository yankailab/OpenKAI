/*
 * ImGUIviewer.cpp
 *
 *  Created on: Jun 4, 2026
 *      Author: Codex
 */

#include "ImGUIviewer.h"

#include "imgui.h"
#include <algorithm>
#include <cfloat>
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

	static bool bFinite(const vFloat3 &v)
	{
		IF_F(!std::isfinite(v.x));
		IF_F(!std::isfinite(v.y));
		IF_F(!std::isfinite(v.z));

		return true;
	}

	static vFloat3 materialColor(const vFloat4 &c)
	{
		return vFloat3(c.x, c.y, c.z);
	}

	static vFloat3 visibleColor(const vFloat3 &c, const vFloat4 &matCol)
	{
		if (c.x <= 0.0f && c.y <= 0.0f && c.z <= 0.0f)
			return materialColor(matCol);

		return c;
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

	void IMGUI_VIEWER_OBJ::reserve(void)
	{
		if (m_nPbuf > 0)
			m_vP.reserve(m_nPbuf);

		if (m_nLbuf > 0)
			m_vL.reserve(m_nLbuf);
	}

	void IMGUI_VIEWER_OBJ::clearGeometry(void)
	{
		m_vP.clear();
		m_vL.clear();
	}

	ImGUIviewer::ImGUIviewer()
	{
		m_pBackend = nullptr;
		m_pTui = nullptr;
		m_bShowPanel = true;
		m_bShowGrid = true;
		m_bAutoBound = true;
		m_sMove = 0.01;
		m_sOrbit = 0.008;
		m_sZoom = 0.1;
		m_pointScale = 1.0;
		m_lineScale = 1.0;
		m_vBgCol.set(0.05, 0.055, 0.06, 1.0);
		m_nPbuf = 200000;
		m_nLbuf = 100000;

		pthread_mutex_init(&m_snapshotMutex, NULL);
	}

	ImGUIviewer::~ImGUIviewer()
	{
		if (m_pTui)
			m_pTui->stop();

		if (m_pBackend)
		{
			m_pBackend->shutdown();
			DEL(m_pBackend);
		}

		DEL(m_pTui);
		m_grPt.release();
		m_grLn.release();
		pthread_mutex_destroy(&m_snapshotMutex);
	}

	bool ImGUIviewer::init(const json &j)
	{
		IF_F(!this->_GeometryViewerBase::init(j));

		jKv(j, "bShowPanel", m_bShowPanel);
		jKv(j, "bShowGrid", m_bShowGrid);
		jKv(j, "bAutoBound", m_bAutoBound);
		jKv(j, "sMove", m_sMove);
		jKv(j, "sOrbit", m_sOrbit);
		jKv(j, "sZoom", m_sZoom);
		jKv(j, "pointScale", m_pointScale);
		jKv(j, "lineScale", m_lineScale);
		jKv<float>(j, "vBgCol", m_vBgCol);

		updateBufferLimitsFromConfig(j);
		updateBufferLimitsFromList(jK(j, "vGeometry"));
		updateBufferLimitsFromList(jK(j, "geometry"));

		m_grPt.release();
		m_grLn.release();
		IF_Le_F(!m_grPt.alloc(m_nPbuf), "Alloc failed with nPbuf: " + i2str(m_nPbuf));
		IF_Le_F(!m_grLn.alloc(m_nLbuf), "Alloc failed with nLbuf: " + i2str(m_nLbuf));

		DEL(m_pTui);
		m_pTui = createThread(jK(j, "threadUI"), "threadUI");
		NULL_F(m_pTui);

		return true;
	}

	bool ImGUIviewer::link(const json &j, ModuleMgr *pM)
	{
		m_vpGb.clear();
		IF_F(!this->_GeometryViewerBase::link(j, pM));
		NULL_F(pM);

		m_vGO.clear();
		for (_GeometryBase *pGb : m_vpGb)
		{
			IF_CONT(!pGb);
			upsertGeometry(pGb, pGb->getName());
		}

		IF_F(!parseGeometryList(jK(j, "vGeometry"), pM));
		IF_F(!parseGeometryList(jK(j, "geometry"), pM));
		IF_F(!parseGeometryNames(j, pM));

		NULL_F(m_pTui);
		IF_F(!m_pTui->link(jK(j, "threadUI"), pM));

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
		IF_F(!this->_GeometryViewerBase::check());
		NULL_F(m_pTui);

		return true;
	}

	void ImGUIviewer::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateAllGeometries();
		}
	}

	void ImGUIviewer::updateAllGeometries(void)
	{
		IF_(!this->_GeometryViewerBase::check());

		vector<IMGUI_VIEWER_OBJ> vFrame;
		vFrame.reserve(m_vpGb.size());

		for (_GeometryBase *pGb : m_vpGb)
		{
			IF_CONT(!pGb);

			const IMGUI_VIEWER_OBJ *pStyle = findObject(pGb, pGb->getName());
			IMGUI_VIEWER_OBJ obj;
			if (pStyle)
				obj = *pStyle;
			else
			{
				obj.m_pGB = pGb;
				obj.m_name = pGb->getName();
			}

			IF_CONT(!obj.m_bVisible);

			obj.m_pGB = pGb;
			if (obj.m_name.empty())
				obj.m_name = pGb->getName();
			obj.clearGeometry();
			obj.reserve();

			collectGeometry(pGb, &obj);

			IF_CONT(obj.m_vP.empty() && obj.m_vL.empty());
			vFrame.push_back(obj);
		}

		snapshotLock();
		m_vDrawGO = vFrame;
		snapshotUnlock();
	}

	void ImGUIviewer::collectGeometry(_GeometryBase *pGb, IMGUI_VIEWER_OBJ *pObj)
	{
		NULL_(pGb);
		NULL_(pObj);

		collectPoints(pObj);
		collectLines(pObj);
	}

	void ImGUIviewer::collectPoints(IMGUI_VIEWER_OBJ *pObj)
	{
		NULL_(pObj);
		NULL_(pObj->m_pGB);

		m_grPt.clear();
		IF_(pObj->m_pGB->get(&m_grPt, m_dTexpire) <= 0);

		int i = 0;
		GEOMETRY_POINT *pGp = nullptr;
		while ((pGp = m_grPt.get(i++)))
		{
			IF_CONT(pGp->m_tStamp == 0);
			IF_CONT(!bFinite(pGp->m_vP));
			if (pObj->m_nPbuf > 0 && (int)pObj->m_vP.size() >= pObj->m_nPbuf)
				break;

			IMGUI_VIEWER_POINT p;
			p.m_vP = pGp->m_vP;
			p.m_vC = visibleColor(pGp->m_vC, pObj->m_matCol);
			p.m_size = std::max(1.0f, (float)pGp->m_size);
			pObj->m_vP.push_back(p);
		}
	}

	void ImGUIviewer::collectLines(IMGUI_VIEWER_OBJ *pObj)
	{
		NULL_(pObj);
		NULL_(pObj->m_pGB);

		m_grLn.clear();
		IF_(pObj->m_pGB->get(&m_grLn, m_dTexpire) <= 0);

		int i = 0;
		GEOMETRY_LINE *pGl = nullptr;
		while ((pGl = m_grLn.get(i++)))
		{
			IF_CONT(pGl->m_tStamp == 0);
			IF_CONT(!bFinite(pGl->m_vPa));
			IF_CONT(!bFinite(pGl->m_vPb));
			if (pObj->m_nLbuf > 0 && (int)pObj->m_vL.size() >= pObj->m_nLbuf)
				break;

			IMGUI_VIEWER_LINE l;
			l.m_vA = pGl->m_vPa;
			l.m_vB = pGl->m_vPb;
			l.m_vC = visibleColor(pGl->m_vC, pObj->m_matCol);
			l.m_width = std::max(1.0f, (float)pGl->m_width);
			pObj->m_vL.push_back(l);
		}
	}

	void ImGUIviewer::updateUI(void)
	{
		m_pBackend = createImGuiViewerBackend();
		if (!m_pBackend || !m_pBackend->init(this->getName(), m_vWinSize.x, m_vWinSize.y, m_bFullScreen))
		{
			if (m_pBackend)
			{
				m_pBackend->shutdown();
				DEL(m_pBackend);
			}

			if (m_pT)
				m_pT->stop();
			if (m_pTui)
				m_pTui->stop();
			return;
		}

		while (m_pTui->bAlive() && !m_pBackend->bClose())
		{
			m_pTui->autoFPS();
			m_pBackend->beginFrame();
			drawUI();

			float c[4] = {m_vBgCol.x, m_vBgCol.y, m_vBgCol.z, m_vBgCol.w};
			m_pBackend->endFrame(c);
		}

		m_pBackend->shutdown();
		DEL(m_pBackend);

		if (m_pT)
			m_pT->stop();
		if (m_pTui)
			m_pTui->stop();
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
		canvasSize.x = std::max(canvasSize.x, 1.0f);
		canvasSize.y = std::max(canvasSize.y, 1.0f);

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
		ImGui::SetNextWindowSize(ImVec2(320, 0), ImGuiCond_Once);
		ImGui::Begin("Viewer", &m_bShowPanel, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Backend: %s", getImGuiViewerBackendName());
		ImGui::Text("Update: %.1f FPS", m_pT ? m_pT->getFPS() : 0.0f);
		ImGui::Text("UI: %.1f FPS", m_pTui ? m_pTui->getFPS() : 0.0f);

		vector<IMGUI_VIEWER_OBJ> vGO;
		copySnapshot(&vGO);

		size_t nP = 0;
		size_t nL = 0;
		for (const IMGUI_VIEWER_OBJ &g : vGO)
		{
			nP += g.m_vP.size();
			nL += g.m_vL.size();
		}

		ImGui::Text("Geometry: %zu", vGO.size());
		ImGui::Text("Points: %zu", nP);
		ImGui::Text("Lines: %zu", nL);
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
		pDraw->PushClipRect(p0, p1, true);

		if (m_bAutoBound && camBound())
			m_bAutoBound = false;

		if (m_bShowGrid)
			drawGrid(vCanvasPos, vCanvasSize);

		vector<IMGUI_VIEWER_OBJ> vGO;
		copySnapshot(&vGO);

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
							   std::max(1.0f, l.m_width * g.m_matLineWidth * m_lineScale));
			}

			for (const IMGUI_VIEWER_POINT &p : g.m_vP)
			{
				vFloat2 vS;
				float d = 0;
				if (!projectPoint(p.m_vP, vCanvasPos, vCanvasSize, &vS, &d))
					continue;

				float r = std::max(1.0f, p.m_size * g.m_matPointSize * m_pointScale);
				pDraw->AddCircleFilled(ImVec2(vS.x, vS.y), r, colU32(p.m_vC, g.m_matCol.w), 8);
			}
		}

		pDraw->PopClipRect();
	}

	void ImGUIviewer::drawGrid(const vFloat2 &vCanvasPos, const vFloat2 &vCanvasSize)
	{
		ImDrawList *pDraw = ImGui::GetWindowDrawList();
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

	void ImGUIviewer::updateCameraControl(const vFloat2 &vCanvasSize)
	{
		if (!ImGui::IsItemHovered())
			return;

		ImGuiIO &io = ImGui::GetIO();
		if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			orbit(io.MouseDelta.x * m_sOrbit, io.MouseDelta.y * m_sOrbit);
		if (ImGui::IsMouseDragging(ImGuiMouseButton_Right) || ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
			pan(io.MouseDelta.x, io.MouseDelta.y, vCanvasSize);
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

	bool ImGUIviewer::parseGeometryList(const json &jg, ModuleMgr *pM)
	{
		NULL_F(pM);
		IF__(jg.is_null(), true);

		auto parseOne = [&](const json &ji) -> bool
		{
			IF__(!ji.is_object(), true);

			string n = "";
			jKv(ji, "_GeometryBase", n);
			IF__(n.empty(), true);

			_GeometryBase *pGB = (_GeometryBase *)(pM->findModule(n));
			IF__(!pGB, true);

			return upsertGeometry(pGB, n, &ji);
		};

		if (jg.is_array())
		{
			for (auto it = jg.begin(); it != jg.end(); it++)
				IF_F(!parseOne(it.value()));

			return true;
		}

		if (jg.is_object())
		{
			if (jg.find("_GeometryBase") != jg.end())
				return parseOne(jg);

			for (auto it = jg.begin(); it != jg.end(); it++)
				IF_F(!parseOne(it.value()));
		}

		return true;
	}

	bool ImGUIviewer::parseGeometryNames(const json &j, ModuleMgr *pM)
	{
		NULL_F(pM);

		vector<string> vGn;
		jKv(j, "vGeometryBase", vGn);
		for (const string &n : vGn)
		{
			_GeometryBase *pGB = (_GeometryBase *)(pM->findModule(n));
			IF_CONT(!pGB);
			IF_F(!upsertGeometry(pGB, n));
		}

		return true;
	}

	bool ImGUIviewer::upsertGeometry(_GeometryBase *pGb, const string &name, const json *pJ)
	{
		NULL_F(pGb);

		bool bLinked = false;
		for (_GeometryBase *pLinked : m_vpGb)
		{
			IF_CONT(!pLinked);
			if (pLinked == pGb || (!name.empty() && pLinked->getName() == name))
			{
				bLinked = true;
				break;
			}
		}

		if (!bLinked)
			m_vpGb.push_back(pGb);

		IMGUI_VIEWER_OBJ *pObj = findObject(pGb, name);
		if (!pObj)
		{
			IMGUI_VIEWER_OBJ obj;
			obj.m_pGB = pGb;
			obj.m_name = name.empty() ? pGb->getName() : name;
			obj.reserve();
			m_vGO.push_back(obj);
			pObj = &m_vGO.back();
		}

		if (pObj->m_name.empty())
			pObj->m_name = name.empty() ? pGb->getName() : name;

		if (pJ)
			applyObjectConfig(pObj, *pJ);

		pObj->reserve();
		return true;
	}

	void ImGUIviewer::applyObjectConfig(IMGUI_VIEWER_OBJ *pObj, const json &j)
	{
		NULL_(pObj);
		IF_(!j.is_object());

		jKv(j, "bVisible", pObj->m_bVisible);
		jKv(j, "nP", pObj->m_nPbuf);
		jKv(j, "nPbuf", pObj->m_nPbuf);
		jKv(j, "nL", pObj->m_nLbuf);
		jKv(j, "nLbuf", pObj->m_nLbuf);
		jKv(j, "matPointSize", pObj->m_matPointSize);
		jKv(j, "matLineWidth", pObj->m_matLineWidth);
		jKv<float>(j, "matCol", pObj->m_matCol);
	}

	IMGUI_VIEWER_OBJ *ImGUIviewer::findObject(_GeometryBase *pGb, const string &name)
	{
		for (IMGUI_VIEWER_OBJ &obj : m_vGO)
		{
			if (obj.m_pGB == pGb)
				return &obj;

			if (!name.empty() && obj.m_name == name)
				return &obj;
		}

		return nullptr;
	}

	const IMGUI_VIEWER_OBJ *ImGUIviewer::findObject(_GeometryBase *pGb, const string &name) const
	{
		for (const IMGUI_VIEWER_OBJ &obj : m_vGO)
		{
			if (obj.m_pGB == pGb)
				return &obj;

			if (!name.empty() && obj.m_name == name)
				return &obj;
		}

		return nullptr;
	}

	void ImGUIviewer::updateBufferLimitsFromConfig(const json &j)
	{
		jKv(j, "nP", m_nPbuf);
		jKv(j, "nPbuf", m_nPbuf);
		jKv(j, "nL", m_nLbuf);
		jKv(j, "nLbuf", m_nLbuf);

		m_nPbuf = std::max(1, m_nPbuf);
		m_nLbuf = std::max(1, m_nLbuf);
	}

	void ImGUIviewer::updateBufferLimitsFromList(const json &jg)
	{
		IF_(jg.is_null());

		auto parseOne = [&](const json &ji)
		{
			IF_(!ji.is_object());

			int n = 0;
			if (jKv(ji, "nP", n) || jKv(ji, "nPbuf", n))
				m_nPbuf = std::max(m_nPbuf, n);

			n = 0;
			if (jKv(ji, "nL", n) || jKv(ji, "nLbuf", n))
				m_nLbuf = std::max(m_nLbuf, n);
		};

		if (jg.is_array())
		{
			for (auto it = jg.begin(); it != jg.end(); it++)
				parseOne(it.value());
		}
		else if (jg.is_object())
		{
			if (jg.find("_GeometryBase") != jg.end())
				parseOne(jg);
			else
			{
				for (auto it = jg.begin(); it != jg.end(); it++)
					parseOne(it.value());
			}
		}

		m_nPbuf = std::max(1, m_nPbuf);
		m_nLbuf = std::max(1, m_nLbuf);
	}

	bool ImGUIviewer::projectPoint(const vFloat3 &vP,
								   const vFloat2 &vCanvasPos,
								   const vFloat2 &vCanvasSize,
								   vFloat2 *pVscreen,
								   float *pDepth)
	{
		NULL_F(pVscreen);
		IF_F(vCanvasSize.x <= 1.0f || vCanvasSize.y <= 1.0f);

		vFloat3 f, r, u;
		getCameraBasis(&f, &r, &u);

		vFloat3 d = {vP.x - m_camPose.m_vEye.x, vP.y - m_camPose.m_vEye.y, vP.z - m_camPose.m_vEye.z};
		float x = vDot(d, r);
		float y = vDot(d, u);
		float z = vDot(d, f);

		float zNear = std::max(0.0001f, m_camProj.m_vNF.x);
		float zFar = m_camProj.m_vNF.y;
		if (zFar <= zNear)
			zFar = FLT_MAX;

		IF_F(z < zNear);
		IF_F(z > zFar);

		float nx = 0.0;
		float ny = 0.0;
		if (m_camProj.m_type == 1)
		{
			float l = m_camProj.m_vLR.x;
			float rr = m_camProj.m_vLR.y;
			float b = m_camProj.m_vBT.x;
			float t = m_camProj.m_vBT.y;
			IF_F(fabs(rr - l) <= 1e-6);
			IF_F(fabs(t - b) <= 1e-6);

			nx = ((x - l) / (rr - l)) * 2.0f - 1.0f;
			ny = ((y - b) / (t - b)) * 2.0f - 1.0f;
		}
		else
		{
			float fov = std::clamp(m_camProj.m_fov, 10.0f, 140.0f) * OK_PI / 180.0;
			float sy = 1.0 / tan(fov * 0.5);
			float sx = sy * vCanvasSize.y / std::max(1.0f, vCanvasSize.x);

			nx = (x * sx) / z;
			ny = (y * sy) / z;
		}

		IF_F(!std::isfinite(nx));
		IF_F(!std::isfinite(ny));
		IF_F(nx < -1.5 || nx > 1.5 || ny < -1.5 || ny > 1.5);

		pVscreen->x = vCanvasPos.x + (nx * 0.5 + 0.5) * vCanvasSize.x;
		pVscreen->y = vCanvasPos.y + (-ny * 0.5 + 0.5) * vCanvasSize.y;

		if (pDepth)
			*pDepth = z;

		return true;
	}

	void ImGUIviewer::getCameraBasis(vFloat3 *pForward, vFloat3 *pRight, vFloat3 *pUp)
	{
		vFloat3 f = vNorm(m_camPose.m_vLookAt - m_camPose.m_vEye);
		if (f.len() <= 1e-6)
			f.set(0, 0, -1);

		vFloat3 r = vNorm(vCross(f, m_camPose.m_vUp));
		if (r.len() <= 1e-6)
			r.set(1, 0, 0);

		vFloat3 u = vNorm(vCross(r, f));
		if (u.len() <= 1e-6)
			u.set(0, 1, 0);

		if (pForward)
			*pForward = f;
		if (pRight)
			*pRight = r;
		if (pUp)
			*pUp = u;
	}

	void ImGUIviewer::orbit(float dYaw, float dPitch)
	{
		vFloat3 v = m_camPose.m_vEye - m_camPose.m_vLookAt;
		float r = std::max(0.01f, v.len());
		float yaw = atan2(v.y, v.x) - dYaw;
		float pitch = asin(std::clamp(v.z / r, -0.99f, 0.99f)) + dPitch;
		pitch = std::clamp(pitch, -1.45f, 1.45f);

		float cp = cos(pitch);
		m_camPose.m_vEye.x = m_camPose.m_vLookAt.x + r * cp * cos(yaw);
		m_camPose.m_vEye.y = m_camPose.m_vLookAt.y + r * cp * sin(yaw);
		m_camPose.m_vEye.z = m_camPose.m_vLookAt.z + r * sin(pitch);
		updateCamPose();
	}

	void ImGUIviewer::pan(float dx, float dy, const vFloat2 &vCanvasSize)
	{
		vFloat3 f, r, u;
		getCameraBasis(&f, &r, &u);

		float d = std::max(0.01f, (m_camPose.m_vEye - m_camPose.m_vLookAt).len());
		float sx = dx / std::max(1.0f, vCanvasSize.x);
		float sy = dy / std::max(1.0f, vCanvasSize.y);
		vFloat3 move = (r * (-sx * d * m_sMove * 100.0)) + (u * (sy * d * m_sMove * 100.0));
		m_camPose.m_vEye += move;
		m_camPose.m_vLookAt += move;
		updateCamPose();
	}

	void ImGUIviewer::zoom(float d)
	{
		vFloat3 v = m_camPose.m_vEye - m_camPose.m_vLookAt;
		float s = std::max(0.05f, 1.0f - d * m_sZoom);
		m_camPose.m_vEye = m_camPose.m_vLookAt + (v * s);
		updateCamPose();
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
			IF_(!bFinite(p));

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

		IF_F(!bFound);

		vFloat3 c = (vMin + vMax) * 0.5;
		float radius = std::max((vMax - vMin).len() * 0.5f, 1.0f);
		vFloat3 f, r, u;
		getCameraBasis(&f, &r, &u);
		m_camPose.m_vLookAt = c + m_vCoR;
		m_camPose.m_vEye = m_camPose.m_vLookAt - (f * (radius * 2.5));
		updateCamPose();

		return true;
	}

	void ImGUIviewer::resetCamPose(void)
	{
		this->_GeometryViewerBase::resetCamPose();
	}

	void ImGUIviewer::setCamPose(const GVIEWER_CAM_POSE &camPose)
	{
		this->_GeometryViewerBase::setCamPose(camPose);
	}

	GVIEWER_CAM_POSE ImGUIviewer::getCamPose(void)
	{
		return this->_GeometryViewerBase::getCamPose();
	}

	void ImGUIviewer::setCamProj(const GVIEWER_CAM_PROJ &camProj)
	{
		this->_GeometryViewerBase::setCamProj(camProj);
	}

	GVIEWER_CAM_PROJ ImGUIviewer::getCamProj(void)
	{
		return this->_GeometryViewerBase::getCamProj();
	}

	void ImGUIviewer::updateCamProj(void)
	{
		IF_(!this->_GeometryViewerBase::check());

		if (m_camProj.m_fov < 1.0f)
			m_camProj.m_fov = 1.0f;
	}

	void ImGUIviewer::updateCamPose(void)
	{
		IF_(!this->_GeometryViewerBase::check());

		if (m_camPose.m_vUp.len() <= 1e-6)
			m_camPose.m_vUp.set(0, 1, 0);
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

/*
 * _ImGUIselectableOctGrid.cpp
 *
 *  Created on: Jun 4, 2026
 *      Author: Codex
 */

#include "_ImGUIselectableOctGrid.h"
#include "../../../Universe/Grid/_SelectableOctGrid.h"

#include "ImGUIglRenderer.h"
#include "imgui.h"
#include <algorithm>
#include <cfloat>
#include <cmath>

#if defined(OKAI_IMGUI_RENDERER_OPENGLES) || defined(OKAI_IMGUI_RENDERER_OPENGL)
#define OKAI_IMGUI_VIEWER_GL 1
#endif

namespace kai
{
	static float vDot(const Vector3f &a, const Vector3f &b)
	{
		return a.dot(b);
	}

	static Vector3f vCross(const Vector3f &a, const Vector3f &b)
	{
		return a.cross(b);
	}

	static Vector3f vNorm(const Vector3f &v)
	{
		float l = sqrt(vDot(v, v));
		if (l <= 1e-6)
			return Vector3f(0, 0, 0);

		return v / l;
	}

	static bool bFinite(const Vector3f &v)
	{
		IF_F(!std::isfinite(v.x()));
		IF_F(!std::isfinite(v.y()));
		IF_F(!std::isfinite(v.z()));

		return true;
	}

	static Vector3f visibleColor(Vector3f c, const Vector4f &matCol)
	{
		if (c.x() <= 0.0f && c.y() <= 0.0f && c.z() <= 0.0f)
			return matCol.head<3>();

		return c;
	}

	static ImU32 colU32(const Vector3f &c)
	{
		return IM_COL32((int)(std::clamp(c.x(), 0.0f, 1.0f) * 255.0f),
						(int)(std::clamp(c.y(), 0.0f, 1.0f) * 255.0f),
						(int)(std::clamp(c.z(), 0.0f, 1.0f) * 255.0f), 255);
	}

	static ImU32 colU32(const Vector4f &c, float alphaScale = 1.0)
	{
		return IM_COL32((int)(std::clamp(c.x(), 0.0f, 1.0f) * 255.0f),
						(int)(std::clamp(c.y(), 0.0f, 1.0f) * 255.0f),
						(int)(std::clamp(c.z(), 0.0f, 1.0f) * 255.0f),
						(int)(std::clamp(std::isfinite(c.w()) ? c.w() : 1.f, 0.f, 1.f) *
							std::clamp(std::isfinite(alphaScale) ? alphaScale : 1.f, 0.f, 1.f) * 255.0f));
	}

	void IMGUI_VIEWER_OBJ::clearGeometry(void)
	{
		m_vP.clear();
		m_vL.clear();
		m_vBox.clear();
	}

	_ImGUIselectableOctGrid::_ImGUIselectableOctGrid()
	{
		m_vBgCol = Vector4f(0.05, 0.055, 0.06, 1.0);
		m_vGLCanvasPos = Vector2f(0, 0);
		m_vGLCanvasSize = Vector2f(1, 1);

		pthread_mutex_init(&m_snapshotMutex, NULL);
	}

	_ImGUIselectableOctGrid::~_ImGUIselectableOctGrid()
	{
		if (m_pTui)
			m_pTui->stop();

		if (m_pBackend)
		{
			m_pBackend->shutdown();
			DEL(m_pBackend);
		}

		DEL(m_pGLRenderer);
		DEL(m_pTui);
		pthread_mutex_destroy(&m_snapshotMutex);
	}

	bool _ImGUIselectableOctGrid::loadConfig(void)
	{
		IF_F(!this->_GeometryViewerBase::loadConfig());
		const json &j = *m_pJ;
		jKv(j, "nCbuf", m_nCbuf);
		IF_Le_F(m_nCbuf < 0, "Invalid nCbuf");

		jKv(j, "bShowPanel", m_bShowPanel);
		jKv(j, "bShowGrid", m_bShowGrid);
		jKv(j, "bAutoBound", m_bAutoBound);
		jKv(j, "sMove", m_sMove);
		jKv(j, "sOrbit", m_sOrbit);
		jKv(j, "sZoom", m_sZoom);
		jKv(j, "pointScale", m_pointScale);
		jKv(j, "lineScale", m_lineScale);
		jKv<float>(j, "vBgCol", m_vBgCol);
		jKv(j, "bGpuRender", m_bGpuRender);

		DEL(m_pTui);
		m_pTui = createThread(jK(*m_pJ, "threadUI"), "threadUI");
		NULL_F(m_pTui);

		return true;
	}

	bool _ImGUIselectableOctGrid::saveConfig(bool bExport)
	{
		IF_F(!_GeometryViewerBase::saveConfig(false));

		json &j = *m_pJ;
		j["nCbuf"] = m_nCbuf;
		j["bShowPanel"] = m_bShowPanel;
		j["bShowGrid"] = m_bShowGrid;
		j["bAutoBound"] = m_bAutoBound;
		j["sMove"] = m_sMove;
		j["sOrbit"] = m_sOrbit;
		j["sZoom"] = m_sZoom;
		j["pointScale"] = m_pointScale;
		j["lineScale"] = m_lineScale;
		j["vBgCol"] = {m_vBgCol.x(), m_vBgCol.y(), m_vBgCol.z(), m_vBgCol.w()};
		j["bGpuRender"] = m_bGpuRender;

		IF_F(m_pTui && !m_pTui->saveConfig(false));

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _ImGUIselectableOctGrid::link(void)
	{
		NULL_F(m_pM);
		IF_F(!this->_GeometryViewerBase::link());
		const json &j = *m_pJ;

		string error;
		IF_Le_F(!m_sources.link(j, m_pM, m_nPbuf, m_nLbuf, m_nCbuf, error), error);

		NULL_F(m_pTui);
		IF_F(!m_pTui->link());

		return true;
	}

	bool _ImGUIselectableOctGrid::start(void)
	{
		NULL_F(m_pT);
		IF_F(!m_pT->startThread(getUpdate, this));

		NULL_F(m_pTui);
		IF_F(!m_pTui->startThread(getUpdateUI, this));

		return true;
	}

	bool _ImGUIselectableOctGrid::check(void)
	{
		IF_F(!this->_GeometryViewerBase::check());
		NULL_F(m_pTui);

		return true;
	}

	void _ImGUIselectableOctGrid::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			updateAllGeometries();
		}
	}

	void _ImGUIselectableOctGrid::updateAllGeometries(void)
	{
		IF_(!this->_GeometryViewerBase::check());
		m_vBuildGO.reserve(m_sources.m_vGeometry.size() + m_sources.m_vGrid.size());
		const uint64_t now = getTns();
		const uint64_t expiry = m_dTexpire && now > m_dTexpire ? now - m_dTexpire : 0;
		size_t count = 0, nPoints = 0, nLines = 0;
		auto prepare = [&](const VIEWER_SOURCE_STYLE &style) -> IMGUI_VIEWER_OBJ *
		{
			if (count == m_vBuildGO.size()) m_vBuildGO.emplace_back();
			auto &object = m_vBuildGO[count];
			object.clearGeometry();
			object.m_name = style.m_name;
			object.m_matCol = style.m_matCol;
			object.m_matPointSize = 2;
			object.m_matLineWidth = style.m_matLineWidth;
			return &object;
		};
		for (const auto &source : m_sources.m_vGeometry)
		{
			if (!source.m_bVisible) continue;
			auto *object = prepare(source);
			object->m_matPointSize = source.m_matPointSize;
			collectGeometry(source, object, expiry);
			if (object->m_vP.empty() && object->m_vL.empty()) continue;
			nPoints += object->m_vP.size();
			nLines += object->m_vL.size();
			++count;
		}
		for (const auto &source : m_sources.m_vGrid)
		{
			if (!source.m_bVisible) continue;
			auto *object = prepare(source);
			collectCells(source, object, expiry);
			if (object->m_vBox.empty()) continue;
			nLines += object->m_vBox.size() * 12;
			++count;
		}
		m_vBuildGO.resize(count);
		snapshotLock();
		m_vDrawGO.swap(m_vBuildGO);
		m_nDrawObjects = count;
		m_nDrawPoints = nPoints;
		m_nDrawLines = nLines;
		m_snapshotVersion++;
		snapshotUnlock();
	}

	void _ImGUIselectableOctGrid::collectGeometry(const VIEWER_GEOMETRY_SOURCE &source, IMGUI_VIEWER_OBJ *pObj, uint64_t expiry)
	{
		collectPoints(source, pObj, expiry);
		collectLines(source, pObj, expiry);
	}

	void _ImGUIselectableOctGrid::collectPoints(const VIEWER_GEOMETRY_SOURCE &source, IMGUI_VIEWER_OBJ *pObj, uint64_t expiry)
	{
		NULL_(pObj);
		NULL_(source.m_pGeometry);
		IF_(m_grPt.m_nT == 0 || source.m_nP == 0);
		pObj->m_vP.reserve(source.m_nP);

		m_grPt.m_iT = 0;
		int nGet = source.m_pGeometry->get(&m_grPt, expiry);
		IF_(nGet <= 0);
		nGet = std::min(nGet, m_grPt.m_nT);

		int i = 0;
		GEOMETRY_POINT *pGp = nullptr;
		while (i < nGet && (pGp = m_grPt.get(i++)))
		{
			IF_CONT(pGp->m_tStamp == 0 || pGp->m_tStamp < expiry);
			IF_CONT(!bFinite(pGp->m_vP));
			if ((int)pObj->m_vP.size() >= source.m_nP)
				break;

			IMGUI_VIEWER_POINT p;
			p.m_vP = pGp->m_vP;
			p.m_vC = visibleColor(pGp->m_vC, source.m_matCol);
			pObj->m_vP.push_back(p);
		}
	}

	void _ImGUIselectableOctGrid::collectLines(const VIEWER_GEOMETRY_SOURCE &source, IMGUI_VIEWER_OBJ *pObj, uint64_t expiry)
	{
		NULL_(pObj);
		NULL_(source.m_pGeometry);
		IF_(m_grLn.m_nT == 0 || source.m_nL == 0);
		pObj->m_vL.reserve(source.m_nL);

		m_grLn.m_iT = 0;
		int nGet = source.m_pGeometry->get(&m_grLn, expiry);
		IF_(nGet <= 0);
		nGet = std::min(nGet, m_grLn.m_nT);

		int i = 0;
		GEOMETRY_LINE *pGl = nullptr;
		while (i < nGet && (pGl = m_grLn.get(i++)))
		{
			IF_CONT(pGl->m_tStamp == 0 || pGl->m_tStamp < expiry);
			IF_CONT(!bFinite(pGl->m_vPa));
			IF_CONT(!bFinite(pGl->m_vPb));
			if ((int)pObj->m_vL.size() >= source.m_nL)
				break;

			IMGUI_VIEWER_LINE l;
			l.m_vA = pGl->m_vPa;
			l.m_vB = pGl->m_vPb;
			l.m_vC = visibleColor(pGl->m_vC, source.m_matCol);
			pObj->m_vL.push_back(l);
		}
	}

	void _ImGUIselectableOctGrid::collectCells(const VIEWER_GRID_SOURCE &source, IMGUI_VIEWER_OBJ *pObj, uint64_t expiry)
	{
		NULL_(pObj);
		NULL_(source.m_pGrid);
		source.m_pGrid->get(&m_cells, expiry, size_t(source.m_nC));
		pObj->m_gridHeader = m_cells.m_header;
		pObj->m_vBox.reserve(m_cells.m_vCell.size());
		for (const auto &cell : m_cells.m_vCell)
		{
			IMGUI_VIEWER_BOX box;
			box.m_ID = cell.id();
			std::array<float, 3> c, size;
			if (!octgridCellBox(m_cells.m_header, box.m_ID, c, size)) continue;
			box.m_vCenter = Vector3f(c[0], c[1], c[2]);
			box.m_vSize = Vector3f(size[0], size[1], size[2]);
			box.m_vC = Vector4f(cell.m_vC[0] / 255.f, cell.m_vC[1] / 255.f, cell.m_vC[2] / 255.f, cell.m_vC[3] / 255.f);
			pObj->m_vBox.push_back(box);
		}
	}

	void _ImGUIselectableOctGrid::updateUI(void)
	{
		m_pBackend = createImGUIbackend();
		if (!m_pBackend || !m_pBackend->init(this->getName(), m_vWinSize.x(), m_vWinSize.y(), m_bFullScreen))
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

		while (m_pTui->bRun() && !m_pBackend->bClose())
		{
			m_pTui->autoFPS();
			m_pBackend->beginFrame();
			drawUI();

			float c[4] = {m_vBgCol.x(), m_vBgCol.y(), m_vBgCol.z(), m_vBgCol.w()};
			m_pBackend->endFrame(c);
		}

		if (m_pGLRenderer)
			m_pGLRenderer->release();
		m_pBackend->shutdown();
		DEL(m_pBackend);

		if (m_pT)
			m_pT->stop();
		if (m_pTui)
			m_pTui->stop();
	}

	void _ImGUIselectableOctGrid::drawUI(void)
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
		updateCameraControl(Vector2f(canvasSize.x, canvasSize.y));
		drawScene(Vector2f(canvasPos.x, canvasPos.y), Vector2f(canvasSize.x, canvasSize.y));
		ImGui::End();

		if (m_bShowPanel)
			drawStatusPanel();
	}

	void _ImGUIselectableOctGrid::drawStatusPanel(void)
	{
		ImGui::SetNextWindowPos(ImVec2(12, 12), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(320, 0), ImGuiCond_Once);
		ImGui::Begin("Viewer", &m_bShowPanel, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Backend: %s", getImGUIbackendName());
		ImGui::Text("Update: %.1f FPS", m_pT ? m_pT->getFPS() : 0.0f);
		ImGui::Text("UI: %.1f FPS", m_pTui ? m_pTui->getFPS() : 0.0f);

		snapshotLock();
		size_t nGO = m_nDrawObjects;
		size_t nP = m_nDrawPoints;
		size_t nL = m_nDrawLines;
		snapshotUnlock();

		ImGui::Text("Render: %s", (m_bGpuRender && m_pGLRenderer && m_pGLRenderer->bReady()) ? "GPU" : "CPU");
		ImGui::Text("Geometry: %zu", nGO);
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
		ImGui::ColorEdit4("Background", m_vBgCol.data());
		ImGui::End();
	}

	void _ImGUIselectableOctGrid::drawScene(const Vector2f &vCanvasPos, const Vector2f &vCanvasSize)
	{
		ImDrawList *pDraw = ImGui::GetWindowDrawList();
		ImVec2 p0(vCanvasPos.x(), vCanvasPos.y());
		ImVec2 p1(vCanvasPos.x() + vCanvasSize.x(), vCanvasPos.y() + vCanvasSize.y());
		pDraw->AddRectFilled(p0, p1, colU32(m_vBgCol));
		pDraw->PushClipRect(p0, p1, true);

		if (m_bAutoBound && camBound())
			m_bAutoBound = false;

		if (m_bShowGrid)
			drawGrid(vCanvasPos, vCanvasSize);

#if defined(OKAI_IMGUI_VIEWER_GL)
		if (m_bGpuRender)
			drawSceneGL(vCanvasPos, vCanvasSize);
		else
			drawSceneCPU(vCanvasPos, vCanvasSize);
#else
		drawSceneCPU(vCanvasPos, vCanvasSize);
#endif

		pDraw->PopClipRect();
	}

	void _ImGUIselectableOctGrid::drawSceneCPU(const Vector2f &vCanvasPos, const Vector2f &vCanvasSize)
	{
		ImDrawList *pDraw = ImGui::GetWindowDrawList();

		snapshotLock();
		for (const IMGUI_VIEWER_OBJ &g : m_vDrawGO)
		{
			auto drawLine = [&](const Vector3f &vA, const Vector3f &vB, ImU32 color)
			{
				Vector2f a = Vector2f::Zero(), b = Vector2f::Zero();
				float dA = 0;
				float dB = 0;
				if (!projectPoint(vA, vCanvasPos, vCanvasSize, &a, &dA))
					return;
				if (!projectPoint(vB, vCanvasPos, vCanvasSize, &b, &dB))
					return;

				pDraw->AddLine(ImVec2(a.x(), a.y()), ImVec2(b.x(), b.y()),
							   color,
							   std::max(1.0f, g.m_matLineWidth * m_lineScale));
			};
			for (const auto &line : g.m_vL) drawLine(line.m_vA, line.m_vB, colU32(line.m_vC));
			for (const auto &box : g.m_vBox)
				box.forEachEdge([&](const Vector3f &a, const Vector3f &b) { drawLine(a, b, colU32(box.m_vC, g.m_matCol.w())); });

			for (const IMGUI_VIEWER_POINT &p : g.m_vP)
			{
				Vector2f vS = Vector2f::Zero();
				float d = 0;
				if (!projectPoint(p.m_vP, vCanvasPos, vCanvasSize, &vS, &d))
					continue;

				float r = std::max(1.0f, g.m_matPointSize * m_pointScale);
				pDraw->AddCircleFilled(ImVec2(vS.x(), vS.y()), r, colU32(p.m_vC), 8);
			}
		}
		snapshotUnlock();
	}

	void _ImGUIselectableOctGrid::drawSceneGL(const Vector2f &vCanvasPos, const Vector2f &vCanvasSize)
	{
		m_vGLCanvasPos = vCanvasPos;
		m_vGLCanvasSize = vCanvasSize;

		ImDrawList *pDraw = ImGui::GetWindowDrawList();
		pDraw->AddCallback(drawSceneGLCallback, this);
		pDraw->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
	}

	void _ImGUIselectableOctGrid::drawSceneGLCallback(const ImDrawList *, const ImDrawCmd *pCmd)
	{
		if (!pCmd || !pCmd->UserCallbackData)
			return;

		_ImGUIselectableOctGrid *pViewer = (_ImGUIselectableOctGrid *)pCmd->UserCallbackData;
		pViewer->renderSceneGL(pViewer->m_vGLCanvasPos, pViewer->m_vGLCanvasSize);
	}

#if defined(OKAI_IMGUI_VIEWER_GL)
	void _ImGUIselectableOctGrid::renderSceneGL(const Vector2f &vCanvasPos, const Vector2f &vCanvasSize)
	{
		if (!m_pGLRenderer)
			m_pGLRenderer = new ImGUIglRenderer();
		NULL_(m_pGLRenderer);

		IMGUI_VIEWER_GL_FRAME frame;
		frame.m_vCanvasPos = vCanvasPos;
		frame.m_vCanvasSize = vCanvasSize;
		frame.m_camPose = m_camPose;
		frame.m_camProj = m_camProj;
		frame.m_pointScale = m_pointScale;
		frame.m_lineScale = m_lineScale;
		getCameraBasis(&frame.m_vForward, &frame.m_vRight, &frame.m_vUp);

		snapshotLock();
		bool bSnapshotReady = m_pGLRenderer->prepareSnapshot(m_vDrawGO,
															 m_nDrawPoints,
															 m_nDrawLines,
															 m_snapshotVersion);
		snapshotUnlock();

		if (!bSnapshotReady || !m_pGLRenderer->render(frame))
			m_bGpuRender = false;
	}
#else
	void _ImGUIselectableOctGrid::renderSceneGL(const Vector2f &, const Vector2f &)
	{
	}
#endif

	void _ImGUIselectableOctGrid::drawGrid(const Vector2f &vCanvasPos, const Vector2f &vCanvasSize)
	{
		ImDrawList *pDraw = ImGui::GetWindowDrawList();
		const float r = 10.0;
		const ImU32 c = IM_COL32(70, 75, 82, 180);

		for (int i = -10; i <= 10; i++)
		{
			Vector2f a = Vector2f::Zero(), b = Vector2f::Zero();
			float dA = 0;
			float dB = 0;
			if (projectPoint(Vector3f(i, -r, 0), vCanvasPos, vCanvasSize, &a, &dA) &&
				projectPoint(Vector3f(i, r, 0), vCanvasPos, vCanvasSize, &b, &dB))
				pDraw->AddLine(ImVec2(a.x(), a.y()), ImVec2(b.x(), b.y()), c, 1.0);

			if (projectPoint(Vector3f(-r, i, 0), vCanvasPos, vCanvasSize, &a, &dA) &&
				projectPoint(Vector3f(r, i, 0), vCanvasPos, vCanvasSize, &b, &dB))
				pDraw->AddLine(ImVec2(a.x(), a.y()), ImVec2(b.x(), b.y()), c, 1.0);
		}
	}

	void _ImGUIselectableOctGrid::updateCameraControl(const Vector2f &vCanvasSize)
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

	void _ImGUIselectableOctGrid::copySnapshot(vector<IMGUI_VIEWER_OBJ> *pVgo)
	{
		NULL_(pVgo);

		snapshotLock();
		*pVgo = m_vDrawGO;
		snapshotUnlock();
	}

	bool _ImGUIselectableOctGrid::projectPoint(const Vector3f &vP,
								   const Vector2f &vCanvasPos,
								   const Vector2f &vCanvasSize,
								   Vector2f *pVscreen,
								   float *pDepth)
	{
		NULL_F(pVscreen);
		IF_F(vCanvasSize.x() <= 1.0f || vCanvasSize.y() <= 1.0f);

		Vector3f f = Vector3f::Zero(), r = Vector3f::Zero(), u = Vector3f::Zero();
		getCameraBasis(&f, &r, &u);

		Vector3f d = {vP.x() - m_camPose.m_vEye.x(), vP.y() - m_camPose.m_vEye.y(), vP.z() - m_camPose.m_vEye.z()};
		float x = vDot(d, r);
		float y = vDot(d, u);
		float z = vDot(d, f);

		float zNear = std::max(0.0001f, m_camProj.m_vNF.x());
		float zFar = m_camProj.m_vNF.y();
		if (zFar <= zNear)
			zFar = FLT_MAX;

		IF_F(z < zNear);
		IF_F(z > zFar);

		float nx = 0.0;
		float ny = 0.0;
		if (m_camProj.m_type == 1)
		{
			float l = m_camProj.m_vLR.x();
			float rr = m_camProj.m_vLR.y();
			float b = m_camProj.m_vBT.x();
			float t = m_camProj.m_vBT.y();
			IF_F(fabs(rr - l) <= 1e-6);
			IF_F(fabs(t - b) <= 1e-6);

			nx = ((x - l) / (rr - l)) * 2.0f - 1.0f;
			ny = ((y - b) / (t - b)) * 2.0f - 1.0f;
		}
		else
		{
			float fov = std::clamp(m_camProj.m_fov, 10.0f, 140.0f) * OK_PI / 180.0;
			float sy = 1.0 / tan(fov * 0.5);
			float sx = sy * vCanvasSize.y() / std::max(1.0f, vCanvasSize.x());

			nx = (x * sx) / z;
			ny = (y * sy) / z;
		}

		IF_F(!std::isfinite(nx));
		IF_F(!std::isfinite(ny));
		IF_F(nx < -1.5 || nx > 1.5 || ny < -1.5 || ny > 1.5);

		pVscreen->x() = vCanvasPos.x() + (nx * 0.5 + 0.5) * vCanvasSize.x();
		pVscreen->y() = vCanvasPos.y() + (-ny * 0.5 + 0.5) * vCanvasSize.y();

		if (pDepth)
			*pDepth = z;

		return true;
	}

	void _ImGUIselectableOctGrid::getCameraBasis(Vector3f *pForward, Vector3f *pRight, Vector3f *pUp)
	{
		Vector3f f = vNorm(m_camPose.m_vLookAt - m_camPose.m_vEye);
		if (f.norm() <= 1e-6)
			f = Vector3f(0, 0, -1);

		Vector3f r = vNorm(vCross(f, m_camPose.m_vUp));
		if (r.norm() <= 1e-6)
			r = Vector3f(1, 0, 0);

		Vector3f u = vNorm(vCross(r, f));
		if (u.norm() <= 1e-6)
			u = Vector3f(0, 1, 0);

		if (pForward)
			*pForward = f;
		if (pRight)
			*pRight = r;
		if (pUp)
			*pUp = u;
	}

	void _ImGUIselectableOctGrid::orbit(float dYaw, float dPitch)
	{
		Vector3f v = m_camPose.m_vEye - m_camPose.m_vLookAt;
		float r = std::max(0.01f, v.norm());
		float yaw = atan2(v.y(), v.x()) - dYaw;
		float pitch = asin(std::clamp(v.z() / r, -0.99f, 0.99f)) + dPitch;
		pitch = std::clamp(pitch, -1.45f, 1.45f);

		float cp = cos(pitch);
		m_camPose.m_vEye.x() = m_camPose.m_vLookAt.x() + r * cp * cos(yaw);
		m_camPose.m_vEye.y() = m_camPose.m_vLookAt.y() + r * cp * sin(yaw);
		m_camPose.m_vEye.z() = m_camPose.m_vLookAt.z() + r * sin(pitch);
		updateCamPose();
	}

	void _ImGUIselectableOctGrid::pan(float dx, float dy, const Vector2f &vCanvasSize)
	{
		Vector3f f = Vector3f::Zero(), r = Vector3f::Zero(), u = Vector3f::Zero();
		getCameraBasis(&f, &r, &u);

		float d = std::max(0.01f, (m_camPose.m_vEye - m_camPose.m_vLookAt).norm());
		float sx = dx / std::max(1.0f, vCanvasSize.x());
		float sy = dy / std::max(1.0f, vCanvasSize.y());
		Vector3f move = (r * (-sx * d * m_sMove * 100.0)) + (u * (sy * d * m_sMove * 100.0));
		m_camPose.m_vEye += move;
		m_camPose.m_vLookAt += move;
		updateCamPose();
	}

	void _ImGUIselectableOctGrid::zoom(float d)
	{
		Vector3f v = m_camPose.m_vEye - m_camPose.m_vLookAt;
		float s = std::max(0.05f, 1.0f - d * m_sZoom);
		m_camPose.m_vEye = m_camPose.m_vLookAt + (v * s);
		updateCamPose();
	}

	bool _ImGUIselectableOctGrid::camBound(void)
	{
		vector<IMGUI_VIEWER_OBJ> vGO;
		copySnapshot(&vGO);

		bool bFound = false;
		Vector3f vMin(FLT_MAX, FLT_MAX, FLT_MAX);
		Vector3f vMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		auto expand = [&](const Vector3f &p)
		{
			IF_(!bFinite(p));

			vMin.x() = std::min(vMin.x(), p.x());
			vMin.y() = std::min(vMin.y(), p.y());
			vMin.z() = std::min(vMin.z(), p.z());
			vMax.x() = std::max(vMax.x(), p.x());
			vMax.y() = std::max(vMax.y(), p.y());
			vMax.z() = std::max(vMax.z(), p.z());
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
			for (const auto &box : g.m_vBox)
			{
				Vector3f center = box.m_vCenter, half = box.m_vSize;
				half *= 0.5f;
				expand(center - half);
				expand(center + half);
			}
		}

		IF_F(!bFound);

		Vector3f c = (vMin + vMax) * 0.5;
		float radius = std::max((vMax - vMin).norm() * 0.5f, 1.0f);
		Vector3f f = Vector3f::Zero(), r = Vector3f::Zero(), u = Vector3f::Zero();
		getCameraBasis(&f, &r, &u);
		m_camPose.m_vLookAt = c + m_vCoR;
		m_camPose.m_vEye = m_camPose.m_vLookAt - (f * (radius * 2.5));
		updateCamPose();

		return true;
	}

	void _ImGUIselectableOctGrid::resetCamPose(void)
	{
		this->_GeometryViewerBase::resetCamPose();
	}

	void _ImGUIselectableOctGrid::setCamPose(const GVIEWER_CAM_POSE &camPose)
	{
		this->_GeometryViewerBase::setCamPose(camPose);
	}

	GVIEWER_CAM_POSE _ImGUIselectableOctGrid::getCamPose(void)
	{
		return this->_GeometryViewerBase::getCamPose();
	}

	void _ImGUIselectableOctGrid::setCamProj(const GVIEWER_CAM_PROJ &camProj)
	{
		this->_GeometryViewerBase::setCamProj(camProj);
	}

	GVIEWER_CAM_PROJ _ImGUIselectableOctGrid::getCamProj(void)
	{
		return this->_GeometryViewerBase::getCamProj();
	}

	void _ImGUIselectableOctGrid::updateCamProj(void)
	{
		IF_(!this->_GeometryViewerBase::check());

		if (m_camProj.m_fov < 1.0f)
			m_camProj.m_fov = 1.0f;
	}

	void _ImGUIselectableOctGrid::updateCamPose(void)
	{
		IF_(!this->_GeometryViewerBase::check());

		if (m_camPose.m_vUp.norm() <= 1e-6)
			m_camPose.m_vUp = Vector3f(0, 1, 0);
	}

	void _ImGUIselectableOctGrid::snapshotLock(void)
	{
		pthread_mutex_lock(&m_snapshotMutex);
	}

	void _ImGUIselectableOctGrid::snapshotUnlock(void)
	{
		pthread_mutex_unlock(&m_snapshotMutex);
	}
}

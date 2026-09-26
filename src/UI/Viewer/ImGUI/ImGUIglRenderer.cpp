/*
 * ImGUIglRenderer.cpp
 *
 *  Created on: Sep 7, 2026
 *      Author: Codex
 */

#include "ImGUIglRenderer.h"

#include "imgui.h"
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstddef>

#if defined(OKAI_IMGUI_RENDERER_OPENGLES)
#include <GLES3/gl3.h>
#define OKAI_IMGUI_VIEWER_GL 1
#elif defined(OKAI_IMGUI_RENDERER_OPENGL)
#include <GL/glew.h>
#define OKAI_IMGUI_VIEWER_GL 1
#endif

namespace kai
{
	ImGUIglRenderer::ImGUIglRenderer()
	{
	}

	ImGUIglRenderer::~ImGUIglRenderer()
	{
	}

	bool ImGUIglRenderer::bReady(void) const
	{
		return m_bReady;
	}

	bool ImGUIglRenderer::prepareSnapshot(const vector<IMGUI_VIEWER_OBJ> &vGO,
												size_t nP,
												size_t nL,
												unsigned long long version)
	{
		IF__(version == m_uploadedVersion || (m_bPendingUpload && version == m_pendingVersion), true);

		m_vPointUpload.clear();
		m_vLineUpload.clear();
		m_vCellUpload.clear();
		m_vPointBatch.clear();
		m_vLineBatch.clear();
		m_vCellBatch.clear();
		m_vDrawCmd.clear();
		m_vPointUpload.reserve(nP);
		size_t cellEdges = 0;
		for (const auto &g : vGO) cellEdges += g.m_vBox.size() * 12;
		m_vLineUpload.reserve((nL > cellEdges ? nL - cellEdges : 0) * 2);
		m_vCellUpload.reserve(cellEdges * 2);

		for (const IMGUI_VIEWER_OBJ &g : vGO)
		{
			if (!g.m_vL.empty())
			{
				float linePx = std::max(1.0f, g.m_matLineWidth);
				DRAW_BATCH b;
				b.m_first = (int)m_vLineUpload.size();
				b.m_renderPx = linePx;
				m_vLineBatch.push_back(b);

				DRAW_CMD cmd;
				cmd.m_bLine = true;
				cmd.m_iBatch = (int)m_vLineBatch.size() - 1;
				m_vDrawCmd.push_back(cmd);

				for (const IMGUI_VIEWER_LINE &l : g.m_vL)
				{
					m_vLineUpload.push_back({l.m_vA.x(), l.m_vA.y(), l.m_vA.z(),
											  l.m_vC.x(), l.m_vC.y(), l.m_vC.z()});
					m_vLineUpload.push_back({l.m_vB.x(), l.m_vB.y(), l.m_vB.z(),
											  l.m_vC.x(), l.m_vC.y(), l.m_vC.z()});
					m_vLineBatch.back().m_count += 2;
				}
			}

			if (!g.m_vBox.empty())
			{
				const float alpha = std::clamp(std::isfinite(g.m_matCol.w()) ? g.m_matCol.w() : 1.f, 0.f, 1.f);
				DRAW_BATCH batch;
				batch.m_first = (int)m_vCellUpload.size();
				batch.m_renderPx = std::max(1.0f, g.m_matLineWidth);
				m_vCellBatch.push_back(batch);
				DRAW_CMD cmd;
				cmd.m_bLine = true;
				cmd.m_bCell = true;
				cmd.m_iBatch = (int)m_vCellBatch.size() - 1;
				m_vDrawCmd.push_back(cmd);
				for (const auto &box : g.m_vBox)
					box.forEachEdge([&](const Vector3f &a, const Vector3f &b) {
						m_vCellUpload.push_back({a.x(), a.y(), a.z(), box.m_vC.x(), box.m_vC.y(), box.m_vC.z(), alpha * box.m_vC.w()});
						m_vCellUpload.push_back({b.x(), b.y(), b.z(), box.m_vC.x(), box.m_vC.y(), box.m_vC.z(), alpha * box.m_vC.w()});
						m_vCellBatch.back().m_count += 2;
					});
			}

			if (!g.m_vP.empty())
			{
				float pointPx = std::max(1.0f, g.m_matPointSize);
				DRAW_BATCH b;
				b.m_first = (int)m_vPointUpload.size();
				b.m_renderPx = pointPx;
				m_vPointBatch.push_back(b);

				DRAW_CMD cmd;
				cmd.m_bLine = false;
				cmd.m_iBatch = (int)m_vPointBatch.size() - 1;
				m_vDrawCmd.push_back(cmd);

				for (const IMGUI_VIEWER_POINT &p : g.m_vP)
				{
					m_vPointUpload.push_back({p.m_vP.x(), p.m_vP.y(), p.m_vP.z(),
											   p.m_vC.x(), p.m_vC.y(), p.m_vC.z()});
					m_vPointBatch.back().m_count++;
				}
			}
		}

		m_pendingVersion = version;
		m_bPendingUpload = true;
		return true;
	}

#if defined(OKAI_IMGUI_VIEWER_GL)
	bool ImGUIglRenderer::render(const IMGUI_VIEWER_GL_FRAME &frame)
	{
		IF_F(!init());

		IF_F(!uploadPreparedSnapshot());

		ImDrawData *pDrawData = ImGui::GetDrawData();
		IF__(!pDrawData, true);

		ImVec2 fbScale = pDrawData->FramebufferScale;
		int fbW = (int)(pDrawData->DisplaySize.x * fbScale.x);
		int fbH = (int)(pDrawData->DisplaySize.y * fbScale.y);
		IF__(fbW <= 0 || fbH <= 0, true);

		int x = (int)((frame.m_vCanvasPos.x() - pDrawData->DisplayPos.x) * fbScale.x);
		int yTop = (int)((frame.m_vCanvasPos.y() - pDrawData->DisplayPos.y) * fbScale.y);
		int w = (int)(frame.m_vCanvasSize.x() * fbScale.x);
		int h = (int)(frame.m_vCanvasSize.y() * fbScale.y);
		IF__(w <= 0 || h <= 0, true);

		if (x < 0)
		{
			w += x;
			x = 0;
		}
		if (yTop < 0)
		{
			h += yTop;
			yTop = 0;
		}
		if (x + w > fbW)
			w = fbW - x;
		if (yTop + h > fbH)
			h = fbH - yTop;
		IF__(w <= 0 || h <= 0, true);

		int y = fbH - yTop - h;

		glViewport(x, y, w, h);
		glScissor(x, y, w, h);
		glEnable(GL_SCISSOR_TEST);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#if !defined(OKAI_IMGUI_RENDERER_OPENGLES)
		glEnable(GL_PROGRAM_POINT_SIZE);
#endif

		glUseProgram(m_program);
		updateCameraUniforms(frame);

		float fbPointScale = std::max(1.0f, std::max(fbScale.x, fbScale.y));

		unsigned int boundVAO = 0;
		for (const DRAW_CMD &cmd : m_vDrawCmd)
		{
			if (cmd.m_bLine)
			{
				const auto &batches = cmd.m_bCell ? m_vCellBatch : m_vLineBatch;
				const auto vao = cmd.m_bCell ? m_vaoC : m_vaoL;
				IF_CONT(cmd.m_iBatch < 0 || cmd.m_iBatch >= (int)batches.size());
				IF_CONT((cmd.m_bCell ? m_nCellVertices : m_nLines) <= 0 || vao == 0);

				const DRAW_BATCH &b = batches[cmd.m_iBatch];
				IF_CONT(b.m_count <= 0);

				if (boundVAO != vao)
				{
					glBindVertexArray(vao);
					boundVAO = vao;
				}
				// Generic attributes are context state and may change after drawing cells.
				if (!cmd.m_bCell && m_attrAlpha >= 0) glVertexAttrib1f((GLuint)m_attrAlpha, 1.0f);

				glUniform1i(m_locRoundPoints, 0);
				glUniform1f(m_locPointScale, 1.0f);
				glLineWidth(std::max(1.0f, b.m_renderPx * frame.m_lineScale * fbPointScale));
				glDrawArrays(GL_LINES, b.m_first, b.m_count);
			}
			else
			{
				IF_CONT(cmd.m_iBatch < 0 || cmd.m_iBatch >= (int)m_vPointBatch.size());
				IF_CONT(m_nPoints <= 0 || m_vaoP == 0);

				const DRAW_BATCH &b = m_vPointBatch[cmd.m_iBatch];
				IF_CONT(b.m_count <= 0);

				if (boundVAO != m_vaoP)
				{
					glBindVertexArray(m_vaoP);
					boundVAO = m_vaoP;
				}
				if (m_attrAlpha >= 0) glVertexAttrib1f((GLuint)m_attrAlpha, 1.0f);

				glUniform1i(m_locRoundPoints, 1);
				glUniform1f(m_locPointScale, std::max(1.0f, b.m_renderPx * frame.m_pointScale * fbPointScale));
				glDrawArrays(GL_POINTS, b.m_first, b.m_count);
			}
		}

		glBindVertexArray(0);
		glUseProgram(0);
		return true;
	}

	bool ImGUIglRenderer::init(void)
	{
		IF__(m_bReady, true);
		IF_F(m_bInitTried);

		m_bInitTried = true;
		m_uploadedVersion = 0;

		if (!compileProgram())
		{
			release();
			return false;
		}

		glGenVertexArrays(1, &m_vaoP);
		glGenBuffers(1, &m_vboP);
		glBindVertexArray(m_vaoP);
		glBindBuffer(GL_ARRAY_BUFFER, m_vboP);
		bindVertexLayout();

		glGenVertexArrays(1, &m_vaoL);
		glGenBuffers(1, &m_vboL);
		glBindVertexArray(m_vaoL);
		glBindBuffer(GL_ARRAY_BUFFER, m_vboL);
		bindVertexLayout();

		glGenVertexArrays(1, &m_vaoC);
		glGenBuffers(1, &m_vboC);
		glBindVertexArray(m_vaoC);
		glBindBuffer(GL_ARRAY_BUFFER, m_vboC);
		bindVertexLayout(true);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		m_bReady = true;
		return true;
	}

	void ImGUIglRenderer::release(void)
	{
		if (m_vboP != 0)
			glDeleteBuffers(1, &m_vboP);
		if (m_vaoP != 0)
			glDeleteVertexArrays(1, &m_vaoP);
		if (m_vboL != 0)
			glDeleteBuffers(1, &m_vboL);
		if (m_vaoL != 0)
			glDeleteVertexArrays(1, &m_vaoL);
		if (m_vboC != 0)
			glDeleteBuffers(1, &m_vboC);
		if (m_vaoC != 0)
			glDeleteVertexArrays(1, &m_vaoC);

		m_vaoP = 0;
		m_vboP = 0;
		m_vaoL = 0;
		m_vboL = 0;
		m_vaoC = 0;
		m_vboC = 0;
		m_vPointUpload.clear();
		m_vLineUpload.clear();
		m_vCellUpload.clear();
		m_vPointBatch.clear();
		m_vLineBatch.clear();
		m_vCellBatch.clear();
		m_vDrawCmd.clear();
		m_nPoints = 0;
		m_nLines = 0;
		m_nCellVertices = 0;
		m_bPendingUpload = false;

		if (m_program != 0)
		{
			glDeleteProgram(m_program);
			m_program = 0;
		}

		m_bReady = false;
		m_bInitTried = false;
		m_uploadedVersion = 0;
		m_pendingVersion = 0;
	}

	bool ImGUIglRenderer::uploadPreparedSnapshot(void)
	{
		IF__(!m_bPendingUpload, true);

		glBindBuffer(GL_ARRAY_BUFFER, m_vboP);
		glBufferData(GL_ARRAY_BUFFER,
					 (GLsizeiptr)(m_vPointUpload.size() * sizeof(VERTEX)),
					 m_vPointUpload.empty() ? nullptr : m_vPointUpload.data(),
					 GL_DYNAMIC_DRAW);
		m_nPoints = (int)m_vPointUpload.size();

		glBindBuffer(GL_ARRAY_BUFFER, m_vboL);
		glBufferData(GL_ARRAY_BUFFER,
					 (GLsizeiptr)(m_vLineUpload.size() * sizeof(VERTEX)),
					 m_vLineUpload.empty() ? nullptr : m_vLineUpload.data(),
					 GL_DYNAMIC_DRAW);
		m_nLines = (int)m_vLineUpload.size();

		glBindBuffer(GL_ARRAY_BUFFER, m_vboC);
		glBufferData(GL_ARRAY_BUFFER,
					 (GLsizeiptr)(m_vCellUpload.size() * sizeof(CELL_VERTEX)),
					 m_vCellUpload.empty() ? nullptr : m_vCellUpload.data(),
					 GL_DYNAMIC_DRAW);
		m_nCellVertices = (int)m_vCellUpload.size();

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		m_uploadedVersion = m_pendingVersion;
		m_bPendingUpload = false;
		return true;
	}

	void ImGUIglRenderer::bindVertexLayout(bool cells)
	{
		const size_t stride = cells ? sizeof(CELL_VERTEX) : sizeof(VERTEX);
		if (m_attrPos >= 0)
		{
			glEnableVertexAttribArray((GLuint)m_attrPos);
			glVertexAttribPointer((GLuint)m_attrPos, 3, GL_FLOAT, GL_FALSE,
								  stride,
								  (void *)offsetof(VERTEX, x));
		}

		if (m_attrCol >= 0)
		{
			glEnableVertexAttribArray((GLuint)m_attrCol);
			glVertexAttribPointer((GLuint)m_attrCol, 3, GL_FLOAT, GL_FALSE,
								  stride,
								  (void *)offsetof(VERTEX, r));
		}

		if (m_attrAlpha >= 0 && cells)
		{
			glEnableVertexAttribArray((GLuint)m_attrAlpha);
			glVertexAttribPointer((GLuint)m_attrAlpha, 1, GL_FLOAT, GL_FALSE,
								  stride,
								  (void *)offsetof(CELL_VERTEX, a));
		}
		else if (m_attrAlpha >= 0)
			glDisableVertexAttribArray((GLuint)m_attrAlpha);

	}

	bool ImGUIglRenderer::compileProgram(void)
	{
#if defined(OKAI_IMGUI_RENDERER_OPENGLES)
		const char *pVs =
			"#version 300 es\n"
			"precision highp float;\n"
			"in vec3 aPos;\n"
			"in vec3 aColor;\n"
			"in float aAlpha;\n"
			"uniform vec3 uEye;\n"
			"uniform vec3 uRight;\n"
			"uniform vec3 uUp;\n"
			"uniform vec3 uForward;\n"
			"uniform vec2 uCanvasSize;\n"
			"uniform int uProjType;\n"
			"uniform float uFovRad;\n"
			"uniform float uNear;\n"
			"uniform float uFar;\n"
			"uniform vec4 uOrtho;\n"
			"uniform float uPointScale;\n"
			"out vec4 vColor;\n"
			"void main() {\n"
			"  vec3 d = aPos - uEye;\n"
			"  float x = dot(d, uRight);\n"
			"  float y = dot(d, uUp);\n"
			"  float z = dot(d, uForward);\n"
			"  if (z < uNear || z > uFar) { gl_Position = vec4(4.0, 4.0, 2.0, 1.0); vColor = vec4(aColor, aAlpha); gl_PointSize = 1.0; return; }\n"
			"  float nx = 0.0;\n"
			"  float ny = 0.0;\n"
			"  if (uProjType == 1) {\n"
			"    nx = ((x - uOrtho.x) / (uOrtho.y - uOrtho.x)) * 2.0 - 1.0;\n"
			"    ny = ((y - uOrtho.z) / (uOrtho.w - uOrtho.z)) * 2.0 - 1.0;\n"
			"  } else {\n"
			"    float sy = 1.0 / tan(uFovRad * 0.5);\n"
			"    float sx = sy * uCanvasSize.y / max(1.0, uCanvasSize.x);\n"
			"    nx = (x * sx) / z;\n"
			"    ny = (y * sy) / z;\n"
			"  }\n"
			"  gl_Position = vec4(nx, ny, 0.0, 1.0);\n"
			"  gl_PointSize = max(1.0, uPointScale);\n"
			"  vColor = vec4(aColor, aAlpha);\n"
			"}\n";

		const char *pFs =
			"#version 300 es\n"
			"precision mediump float;\n"
			"in vec4 vColor;\n"
			"uniform int uRoundPoints;\n"
			"out vec4 fragColor;\n"
			"void main() {\n"
			"  if (uRoundPoints != 0) {\n"
			"    vec2 p = gl_PointCoord * 2.0 - 1.0;\n"
			"    if (dot(p, p) > 1.0) discard;\n"
			"  }\n"
			"  fragColor = vColor;\n"
			"}\n";
#else
		const char *pVs =
			"#version 130\n"
			"in vec3 aPos;\n"
			"in vec3 aColor;\n"
			"in float aAlpha;\n"
			"uniform vec3 uEye;\n"
			"uniform vec3 uRight;\n"
			"uniform vec3 uUp;\n"
			"uniform vec3 uForward;\n"
			"uniform vec2 uCanvasSize;\n"
			"uniform int uProjType;\n"
			"uniform float uFovRad;\n"
			"uniform float uNear;\n"
			"uniform float uFar;\n"
			"uniform vec4 uOrtho;\n"
			"uniform float uPointScale;\n"
			"out vec4 vColor;\n"
			"void main() {\n"
			"  vec3 d = aPos - uEye;\n"
			"  float x = dot(d, uRight);\n"
			"  float y = dot(d, uUp);\n"
			"  float z = dot(d, uForward);\n"
			"  if (z < uNear || z > uFar) { gl_Position = vec4(4.0, 4.0, 2.0, 1.0); vColor = vec4(aColor, aAlpha); gl_PointSize = 1.0; return; }\n"
			"  float nx = 0.0;\n"
			"  float ny = 0.0;\n"
			"  if (uProjType == 1) {\n"
			"    nx = ((x - uOrtho.x) / (uOrtho.y - uOrtho.x)) * 2.0 - 1.0;\n"
			"    ny = ((y - uOrtho.z) / (uOrtho.w - uOrtho.z)) * 2.0 - 1.0;\n"
			"  } else {\n"
			"    float sy = 1.0 / tan(uFovRad * 0.5);\n"
			"    float sx = sy * uCanvasSize.y / max(1.0, uCanvasSize.x);\n"
			"    nx = (x * sx) / z;\n"
			"    ny = (y * sy) / z;\n"
			"  }\n"
			"  gl_Position = vec4(nx, ny, 0.0, 1.0);\n"
			"  gl_PointSize = max(1.0, uPointScale);\n"
			"  vColor = vec4(aColor, aAlpha);\n"
			"}\n";

		const char *pFs =
			"#version 130\n"
			"in vec4 vColor;\n"
			"uniform int uRoundPoints;\n"
			"out vec4 fragColor;\n"
			"void main() {\n"
			"  if (uRoundPoints != 0) {\n"
			"    vec2 p = gl_PointCoord * 2.0 - 1.0;\n"
			"    if (dot(p, p) > 1.0) discard;\n"
			"  }\n"
			"  fragColor = vColor;\n"
			"}\n";
#endif

		unsigned int vs = 0;
		unsigned int fs = 0;
		IF_F(!compileShader(GL_VERTEX_SHADER, pVs, &vs));
		if (!compileShader(GL_FRAGMENT_SHADER, pFs, &fs))
		{
			glDeleteShader(vs);
			return false;
		}

		unsigned int prog = glCreateProgram();
		glAttachShader(prog, vs);
		glAttachShader(prog, fs);
		glLinkProgram(prog);

		glDeleteShader(vs);
		glDeleteShader(fs);

		GLint status = GL_FALSE;
		glGetProgramiv(prog, GL_LINK_STATUS, &status);
		if (status != GL_TRUE)
		{
			char log[1024] = {0};
			glGetProgramInfoLog(prog, sizeof(log) - 1, nullptr, log);
			LOG_("_ImGUIselectableOctGrid GL program link failed: " + string(log));
			glDeleteProgram(prog);
			return false;
		}

		m_program = prog;
		m_attrPos = glGetAttribLocation(m_program, "aPos");
		m_attrCol = glGetAttribLocation(m_program, "aColor");
		m_attrAlpha = glGetAttribLocation(m_program, "aAlpha");
		m_locEye = glGetUniformLocation(m_program, "uEye");
		m_locRight = glGetUniformLocation(m_program, "uRight");
		m_locUp = glGetUniformLocation(m_program, "uUp");
		m_locForward = glGetUniformLocation(m_program, "uForward");
		m_locCanvasSize = glGetUniformLocation(m_program, "uCanvasSize");
		m_locProjType = glGetUniformLocation(m_program, "uProjType");
		m_locFovRad = glGetUniformLocation(m_program, "uFovRad");
		m_locNear = glGetUniformLocation(m_program, "uNear");
		m_locFar = glGetUniformLocation(m_program, "uFar");
		m_locOrtho = glGetUniformLocation(m_program, "uOrtho");
		m_locPointScale = glGetUniformLocation(m_program, "uPointScale");
		m_locRoundPoints = glGetUniformLocation(m_program, "uRoundPoints");

		IF_F(m_attrPos < 0);
		IF_F(m_attrCol < 0);
		IF_F(m_attrAlpha < 0);

		return true;
	}

	bool ImGUIglRenderer::compileShader(unsigned int type, const char *pSrc, unsigned int *pShader)
	{
		NULL_F(pSrc);
		NULL_F(pShader);

		unsigned int shader = glCreateShader((GLenum)type);
		glShaderSource(shader, 1, &pSrc, nullptr);
		glCompileShader(shader);

		GLint status = GL_FALSE;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if (status != GL_TRUE)
		{
			char log[1024] = {0};
			glGetShaderInfoLog(shader, sizeof(log) - 1, nullptr, log);
			LOG_("_ImGUIselectableOctGrid GL shader compile failed: " + string(log));
			glDeleteShader(shader);
			return false;
		}

		*pShader = shader;
		return true;
	}

	void ImGUIglRenderer::updateCameraUniforms(const IMGUI_VIEWER_GL_FRAME &frame)
	{
		float zNear = std::max(0.0001f, frame.m_camProj.m_vNF.x());
		float zFar = frame.m_camProj.m_vNF.y();
		if (!std::isfinite(zFar) || zFar <= zNear || zFar > 1.0e20f)
			zFar = 1.0e20f;

		float fov = std::clamp(frame.m_camProj.m_fov, 10.0f, 140.0f) * OK_PI / 180.0f;
		float l = frame.m_camProj.m_vLR.x();
		float r = frame.m_camProj.m_vLR.y();
		float b = frame.m_camProj.m_vBT.x();
		float t = frame.m_camProj.m_vBT.y();
		if (fabs(r - l) <= 1e-6)
			r = l + 1.0f;
		if (fabs(t - b) <= 1e-6)
			t = b + 1.0f;

		glUniform3f(m_locEye, frame.m_camPose.m_vEye.x(), frame.m_camPose.m_vEye.y(), frame.m_camPose.m_vEye.z());
		glUniform3f(m_locRight, frame.m_vRight.x(), frame.m_vRight.y(), frame.m_vRight.z());
		glUniform3f(m_locUp, frame.m_vUp.x(), frame.m_vUp.y(), frame.m_vUp.z());
		glUniform3f(m_locForward, frame.m_vForward.x(), frame.m_vForward.y(), frame.m_vForward.z());
		glUniform2f(m_locCanvasSize, std::max(1.0f, frame.m_vCanvasSize.x()), std::max(1.0f, frame.m_vCanvasSize.y()));
		glUniform1i(m_locProjType, frame.m_camProj.m_type);
		glUniform1f(m_locFovRad, fov);
		glUniform1f(m_locNear, zNear);
		glUniform1f(m_locFar, zFar);
		glUniform4f(m_locOrtho, l, r, b, t);
	}
#else
	bool ImGUIglRenderer::render(const IMGUI_VIEWER_GL_FRAME &)
	{
		return false;
	}

	bool ImGUIglRenderer::init(void)
	{
		return false;
	}

	void ImGUIglRenderer::release(void)
	{
		m_vPointUpload.clear();
		m_vLineUpload.clear();
		m_vCellUpload.clear();
		m_vPointBatch.clear();
		m_vLineBatch.clear();
		m_vCellBatch.clear();
		m_bReady = false;
		m_bInitTried = false;
		m_bPendingUpload = false;
		m_uploadedVersion = 0;
		m_pendingVersion = 0;
		m_nPoints = 0;
		m_nLines = 0;
		m_nCellVertices = 0;
	}

	bool ImGUIglRenderer::uploadPreparedSnapshot(void)
	{
		return false;
	}

	void ImGUIglRenderer::bindVertexLayout(bool)
	{
	}

	bool ImGUIglRenderer::compileProgram(void)
	{
		return false;
	}

	bool ImGUIglRenderer::compileShader(unsigned int, const char *, unsigned int *)
	{
		return false;
	}

	void ImGUIglRenderer::updateCameraUniforms(const IMGUI_VIEWER_GL_FRAME &)
	{
	}
#endif
}

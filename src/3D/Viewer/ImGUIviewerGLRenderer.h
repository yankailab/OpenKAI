/*
 * ImGUIviewerGLRenderer.h
 *
 *  Created on: Sep 7, 2026
 *      Author: Codex
 */

#ifndef OpenKAI_src_3D_Viewer_ImGUIviewerGLRenderer_H_
#define OpenKAI_src_3D_Viewer_ImGUIviewerGLRenderer_H_

#include "ImGUIviewer.h"

namespace kai
{
	struct IMGUI_VIEWER_GL_FRAME
	{
		vFloat2 m_vCanvasPos;
		vFloat2 m_vCanvasSize;
		GVIEWER_CAM_POSE m_camPose;
		GVIEWER_CAM_PROJ m_camProj;
		vFloat3 m_vForward;
		vFloat3 m_vRight;
		vFloat3 m_vUp;
		float m_pointScale = 1.0f;
		float m_lineScale = 1.0f;
	};

	class ImGUIviewerGLRenderer
	{
	public:
		ImGUIviewerGLRenderer();
		~ImGUIviewerGLRenderer();

		bool bReady(void) const;
		void release(void);

		bool prepareSnapshot(const vector<IMGUI_VIEWER_OBJ> &vGO,
							 size_t nP,
							 size_t nL,
							 unsigned long long version);
		bool render(const IMGUI_VIEWER_GL_FRAME &frame);

	private:
		struct VERTEX
		{
			float x, y, z;
			float r, g, b, a;
		};

		struct DRAW_BATCH
		{
			int m_first = 0;
			int m_count = 0;
			float m_renderPx = 1.0f;
		};

		bool init(void);
		bool uploadPreparedSnapshot(void);
		void bindVertexLayout(void);
		bool compileProgram(void);
		bool compileShader(unsigned int type, const char *pSrc, unsigned int *pShader);
		void updateCameraUniforms(const IMGUI_VIEWER_GL_FRAME &frame);

	private:
		bool m_bInitTried;
		bool m_bReady;
		bool m_bPendingUpload;
		unsigned int m_program;
		unsigned int m_vaoP;
		unsigned int m_vboP;
		unsigned int m_vaoL;
		unsigned int m_vboL;
		int m_attrPos;
		int m_attrCol;
		int m_attrAlpha;
		int m_locEye;
		int m_locRight;
		int m_locUp;
		int m_locForward;
		int m_locCanvasSize;
		int m_locProjType;
		int m_locFovRad;
		int m_locNear;
		int m_locFar;
		int m_locOrtho;
		int m_locPointScale;
		int m_locRoundPoints;
		unsigned long long m_uploadedVersion;
		unsigned long long m_pendingVersion;
		int m_nPoints;
		int m_nLines;
		vector<VERTEX> m_vPointUpload;
		vector<VERTEX> m_vLineUpload;
		vector<DRAW_BATCH> m_vPointBatch;
		vector<DRAW_BATCH> m_vLineBatch;
	};
}

#endif

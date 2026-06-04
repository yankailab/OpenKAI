/*
 * ImGuiViewerBackend.h
 *
 *  Created on: Jun 4, 2026
 *      Author: Codex
 */

#ifndef OpenKAI_src_3D_Viewer_ImGuiViewerBackend_H_
#define OpenKAI_src_3D_Viewer_ImGuiViewerBackend_H_

#include <string>

namespace kai
{
	class ImGuiViewerBackend
	{
	public:
		virtual ~ImGuiViewerBackend() {}

		virtual bool init(const std::string &title, int w, int h, bool bFullScreen) = 0;
		virtual void shutdown(void) = 0;
		virtual bool bClose(void) = 0;
		virtual void beginFrame(void) = 0;
		virtual void endFrame(const float clearCol[4]) = 0;
		virtual void getFramebufferSize(int *pW, int *pH) = 0;
	};

	ImGuiViewerBackend *createImGuiViewerBackend(void);
	const char *getImGuiViewerBackendName(void);
}

#endif

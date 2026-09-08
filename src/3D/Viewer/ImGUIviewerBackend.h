/*
 * ImGUIviewerBackend.h
 *
 *  Created on: Jun 4, 2026
 *      Author: Codex
 */

#ifndef OpenKAI_src_3D_Viewer_ImGUIviewerBackend_H_
#define OpenKAI_src_3D_Viewer_ImGUIviewerBackend_H_

#include <string>

namespace kai
{
	class ImGUIviewerBackend
	{
	public:
		virtual ~ImGUIviewerBackend() {}

		virtual bool init(const std::string &title, int w, int h, bool bFullScreen) = 0;
		virtual void shutdown(void) = 0;
		virtual bool bClose(void) const = 0;
		virtual void beginFrame(void) = 0;
		virtual void endFrame(const float clearCol[4]) = 0;
		virtual void getFramebufferSize(int *pW, int *pH) const = 0;
	};

	ImGUIviewerBackend *createImGUIviewerBackend(void);
	const char *getImGUIviewerBackendName(void);
}

#endif

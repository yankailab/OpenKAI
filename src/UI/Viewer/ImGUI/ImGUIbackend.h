/*
 * ImGUIbackend.h
 *
 *  Created on: Jun 4, 2026
 *      Author: Codex
 */

#ifndef OpenKAI_src_UI_Viewer_ImGUI_ImGUIbackend_H_
#define OpenKAI_src_UI_Viewer_ImGUI_ImGUIbackend_H_

#include <string>

namespace kai
{
	class ImGUIbackend
	{
	public:
		virtual ~ImGUIbackend() {}

		virtual bool init(const std::string &title, int w, int h, bool bFullScreen) = 0;
		virtual void shutdown(void) = 0;
		virtual bool bClose(void) const = 0;
		virtual void beginFrame(void) = 0;
		virtual void endFrame(const float clearCol[4]) = 0;
		virtual void getFramebufferSize(int *pW, int *pH) const = 0;
	};

	ImGUIbackend *createImGUIbackend(void);
	const char *getImGUIbackendName(void);
}

#endif

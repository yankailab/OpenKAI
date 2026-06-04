/*
 * ImGuiViewerBackend.cpp
 *
 *  Created on: Jun 4, 2026
 *      Author: Codex
 */

#include "ImGuiViewerBackend.h"

#ifdef OKAI_IMGUI_BACKEND_GLFW
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#if defined(OKAI_IMGUI_RENDERER_OPENGLES)
#include <GLES3/gl3.h>
#else
#include <GL/gl.h>
#endif

namespace kai
{
	class ImGuiViewerBackendGLFW : public ImGuiViewerBackend
	{
	public:
		ImGuiViewerBackendGLFW()
		{
			m_pWin = nullptr;
		}

		virtual bool init(const std::string &title, int w, int h, bool bFullScreen)
		{
			if (!glfwInit())
				return false;

#if defined(OKAI_IMGUI_RENDERER_OPENGLES)
			glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
			m_glslVersion = "#version 300 es";
#else
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
			m_glslVersion = "#version 130";
#endif

			GLFWmonitor *pMonitor = bFullScreen ? glfwGetPrimaryMonitor() : nullptr;
			m_pWin = glfwCreateWindow(w, h, title.c_str(), pMonitor, nullptr);
			if (!m_pWin)
			{
				glfwTerminate();
				return false;
			}

			glfwMakeContextCurrent(m_pWin);
			glfwSwapInterval(1);

			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGui::StyleColorsDark();

			if (!ImGui_ImplGlfw_InitForOpenGL(m_pWin, true))
				return false;
			if (!ImGui_ImplOpenGL3_Init(m_glslVersion.c_str()))
				return false;

			return true;
		}

		virtual void shutdown(void)
		{
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			if (ImGui::GetCurrentContext())
				ImGui::DestroyContext();

			if (m_pWin)
			{
				glfwDestroyWindow(m_pWin);
				m_pWin = nullptr;
			}
			glfwTerminate();
		}

		virtual bool bClose(void)
		{
			return !m_pWin || glfwWindowShouldClose(m_pWin);
		}

		virtual void beginFrame(void)
		{
			glfwPollEvents();
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
		}

		virtual void endFrame(const float clearCol[4])
		{
			ImGui::Render();

			int w = 0;
			int h = 0;
			glfwGetFramebufferSize(m_pWin, &w, &h);
			glViewport(0, 0, w, h);
			glClearColor(clearCol[0], clearCol[1], clearCol[2], clearCol[3]);
			glClear(GL_COLOR_BUFFER_BIT);
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			glfwSwapBuffers(m_pWin);
		}

		virtual void getFramebufferSize(int *pW, int *pH)
		{
			if (!m_pWin)
				return;

			glfwGetFramebufferSize(m_pWin, pW, pH);
		}

	private:
		GLFWwindow *m_pWin;
		std::string m_glslVersion;
	};

	ImGuiViewerBackend *createImGuiViewerBackend(void)
	{
		return new ImGuiViewerBackendGLFW();
	}

	const char *getImGuiViewerBackendName(void)
	{
#if defined(OKAI_IMGUI_RENDERER_OPENGLES)
		return "GLFW + OpenGL ES";
#else
		return "GLFW + OpenGL";
#endif
	}
}

#elif defined(OKAI_IMGUI_BACKEND_SDL)
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <SDL.h>
#if defined(OKAI_IMGUI_RENDERER_OPENGLES)
#include <GLES3/gl3.h>
#else
#include <GL/gl.h>
#endif

namespace kai
{
	class ImGuiViewerBackendSDL : public ImGuiViewerBackend
	{
	public:
		ImGuiViewerBackendSDL()
		{
			m_pWin = nullptr;
			m_glCtx = nullptr;
			m_bClose = false;
		}

		virtual bool init(const std::string &title, int w, int h, bool bFullScreen)
		{
			if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
				return false;

#if defined(OKAI_IMGUI_RENDERER_OPENGLES)
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
			m_glslVersion = "#version 300 es";
#else
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
			m_glslVersion = "#version 130";
#endif
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

			Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
			if (bFullScreen)
				flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

			m_pWin = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, flags);
			if (!m_pWin)
			{
				SDL_Quit();
				return false;
			}

			m_glCtx = SDL_GL_CreateContext(m_pWin);
			if (!m_glCtx)
				return false;

			SDL_GL_MakeCurrent(m_pWin, m_glCtx);
			SDL_GL_SetSwapInterval(1);

			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGui::StyleColorsDark();

			if (!ImGui_ImplSDL2_InitForOpenGL(m_pWin, m_glCtx))
				return false;
			if (!ImGui_ImplOpenGL3_Init(m_glslVersion.c_str()))
				return false;

			return true;
		}

		virtual void shutdown(void)
		{
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplSDL2_Shutdown();
			if (ImGui::GetCurrentContext())
				ImGui::DestroyContext();

			if (m_glCtx)
			{
				SDL_GL_DeleteContext(m_glCtx);
				m_glCtx = nullptr;
			}
			if (m_pWin)
			{
				SDL_DestroyWindow(m_pWin);
				m_pWin = nullptr;
			}
			SDL_Quit();
		}

		virtual bool bClose(void)
		{
			return m_bClose;
		}

		virtual void beginFrame(void)
		{
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				ImGui_ImplSDL2_ProcessEvent(&event);
				if (event.type == SDL_QUIT)
					m_bClose = true;
				if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE)
					m_bClose = true;
			}

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplSDL2_NewFrame();
			ImGui::NewFrame();
		}

		virtual void endFrame(const float clearCol[4])
		{
			ImGui::Render();

			int w = 0;
			int h = 0;
			SDL_GL_GetDrawableSize(m_pWin, &w, &h);
			glViewport(0, 0, w, h);
			glClearColor(clearCol[0], clearCol[1], clearCol[2], clearCol[3]);
			glClear(GL_COLOR_BUFFER_BIT);
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			SDL_GL_SwapWindow(m_pWin);
		}

		virtual void getFramebufferSize(int *pW, int *pH)
		{
			if (!m_pWin)
				return;

			SDL_GL_GetDrawableSize(m_pWin, pW, pH);
		}

	private:
		SDL_Window *m_pWin;
		SDL_GLContext m_glCtx;
		std::string m_glslVersion;
		bool m_bClose;
	};

	ImGuiViewerBackend *createImGuiViewerBackend(void)
	{
		return new ImGuiViewerBackendSDL();
	}

	const char *getImGuiViewerBackendName(void)
	{
#if defined(OKAI_IMGUI_RENDERER_OPENGLES)
		return "SDL2 + OpenGL ES";
#else
		return "SDL2 + OpenGL";
#endif
	}
}

#else
namespace kai
{
	ImGuiViewerBackend *createImGuiViewerBackend(void)
	{
		return nullptr;
	}

	const char *getImGuiViewerBackendName(void)
	{
		return "none";
	}
}
#endif

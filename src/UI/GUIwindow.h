/*
 * GUIwindow.h
 *
 *  Created on: Mar 26, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_UI_GUIwindow_H_
#define OpenKAI_src_UI_GUIwindow_H_
#ifdef USE_GUI
#include "../Base/_ModuleBase.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_KEYSTATE_BASED_INPUT
//#define NK_IMPLEMENTATION
//#define NK_GLFW_GL3_IMPLEMENTATION
#include "../../GUI/nuklear.h"
#include "../../GUI/backend/glfw_opengl3/nuklear_glfw_gl3.h"

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

#ifdef INCLUDE_ALL
  #define INCLUDE_STYLE
  #define INCLUDE_CALCULATOR
  #define INCLUDE_OVERVIEW
  #define INCLUDE_NODE_EDITOR
#endif

#ifdef INCLUDE_STYLE
  #include "../../GUI/backend/style.c"
#endif
#ifdef INCLUDE_CALCULATOR
  #include "../../GUI/backend/calculator.c"
#endif
#ifdef INCLUDE_OVERVIEW
  #include "../../GUI/backend/overview.c"
#endif
#ifdef INCLUDE_NODE_EDITOR
  #include "../../GUI/backend/node_editor.c"
#endif

/* ===============================================================
 *
 *                          DEMO
 *
 * ===============================================================*/
static void error_callback(int e, const char *d)
{printf("Error %d: %s\n", e, d);}

namespace kai
{

class GUIwindow: public _ModuleBase
{
public:
    GUIwindow();
    ~GUIwindow();

    virtual bool init ( void* pKiss );
    virtual int check ( void );
    virtual bool start ( void );
    virtual void draw ( void );

protected:
    bool setup(void);
//    static void CbRecvData(LivoxEthPacket* pData, void* pLivox);
    
    bool updateGUI ( void );
    void update ( void );
    static void* getUpdate ( void* This )
    {
        ( ( GUIwindow* ) This )->update();
        return NULL;
    }
    
public:

};

}
#endif
#endif

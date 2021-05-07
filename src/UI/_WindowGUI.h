/*
 * _WindowGUI.h
 *
 *  Created on: Mar 26, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_UI_WindowGUI_H_
#define OpenKAI_src_UI_WindowGUI_H_
#ifdef USE_GUI
#include "../Base/_ModuleBase.h"

namespace kai
{

class _WindowGUI: public _ModuleBase
{
public:
    _WindowGUI();
    ~_WindowGUI();

    virtual bool init ( void* pKiss );
    virtual int check ( void );
    virtual bool start ( void );
    virtual void cvDraw(void* pWindow);

protected:
    bool setup(void);
    bool updateGUI ( void );
    void update ( void );
    static void* getUpdate ( void* This )
    {
        ( ( _WindowGUI* ) This )->update();
        return NULL;
    }
    
public:

};

}
#endif
#endif

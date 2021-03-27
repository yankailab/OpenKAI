/*
 * _GUIwindow.h
 *
 *  Created on: Mar 26, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_UI_GUIwindow_H_
#define OpenKAI_src_UI_GUIwindow_H_
#ifdef USE_GUI
#include "../Base/_ModuleBase.h"

namespace kai
{

class _GUIwindow: public _ModuleBase
{
public:
    _GUIwindow();
    ~_GUIwindow();

    virtual bool init ( void* pKiss );
    virtual int check ( void );
    virtual bool start ( void );
    virtual void draw ( void );

protected:
    bool setup(void);
    bool updateGUI ( void );
    void update ( void );
    static void* getUpdate ( void* This )
    {
        ( ( _GUIwindow* ) This )->update();
        return NULL;
    }
    
public:

};

}
#endif
#endif

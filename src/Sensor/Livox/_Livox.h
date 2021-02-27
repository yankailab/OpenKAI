/*
 * _Livox.h
 *
 *  Created on: Aug 21, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Sensor__Livox_H_
#define OpenKAI_src_Sensor__Livox_H_

#include "../../Base/_ModuleBase.h"

#ifdef USE_LIVOX
#include "lds_lidar.h"

namespace kai
{

class _Livox: public _ModuleBase
{
public:
    _Livox();
    ~_Livox();

    bool init ( void* pKiss );
    void close ( void );
    bool start ( void );
    void draw ( void );

private:
    bool open ( void );
    bool updateLidar ( void );
    void update ( void );
    static void* getUpdate ( void* This )
    {
        ( ( _Livox* ) This )->update();
        return NULL;
    }

public:
    bool m_bOpen;
    vector<string> m_vBroadcastCode;

};

}
#endif
#endif

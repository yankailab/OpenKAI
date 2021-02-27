/*
 *  Created on: Aug 21, 2019
 *      Author: yankai
 */
#include "_Livox.h"

#ifdef USE_LIVOX

namespace kai
{

_Livox::_Livox()
{
    m_bOpen = false;
}

_Livox::~_Livox()
{
}

bool _Livox::init ( void* pKiss )
{
    IF_F ( !this->_ModuleBase::init ( pKiss ) );
    Kiss* pK = ( Kiss* ) pKiss;

    pK->a("vBroadcastCode", &m_vBroadcastCode );

    return true;
}

bool _Livox::open ( void )
{

    return true;
}

void _Livox::close ( void )
{
}

bool _Livox::start ( void )
{
    NULL_F ( m_pT );
    return m_pT->start ( getUpdate, this );
}

void _Livox::update ( void )
{
    while ( m_pT->bRun() )
    {
        if ( !m_bOpen )
        {
            if ( !open() )
            {
                m_pT->sleepT ( USEC_1SEC );
                continue;
            }
        }

        m_pT->autoFPSfrom();

        updateLidar();

        m_pT->autoFPSto();
    }
}

bool _Livox::updateLidar ( void )
{
    return true;
}

void _Livox::draw ( void )
{
    this->_ModuleBase::draw();
    string msg;

}

}
#endif


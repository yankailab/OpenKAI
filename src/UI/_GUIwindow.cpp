/*
 *  Created on: Mar 26, 2021
 *      Author: yankai
 */
#ifdef USE_GUI
#include "_GUIwindow.h"
#include "../Script/Kiss.h"

namespace kai
{

    _GUIwindow::_GUIwindow()
    {
    }

    _GUIwindow::~_GUIwindow()
    {
    }

    bool _GUIwindow::init(void *pKiss)
    {
        IF_F(!this->_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        return true;
    }

    bool _GUIwindow::setup(void)
    {
        return true;
    }

    int _GUIwindow::check(void)
    {
        return this->_ModuleBase::check();
    }

    bool _GUIwindow::start ( void )
    {
        NULL_F ( m_pT );
        return m_pT->start ( getUpdate, this );
    }

    void _GUIwindow::update ( void )
    {
        sleep(5);
        setup();

        while ( m_pT->bRun() )
        {
            m_pT->autoFPSfrom();

            updateGUI();

            m_pT->autoFPSto();
        }
    }

    bool _GUIwindow::updateGUI ( void )
    {
        return true;
    }

    // void _GUIwindow::CbRecvData ( LivoxEthPacket* pData, void* pLivox )
    // {
    // }

    void _GUIwindow::draw(void)
    {
        this->_ModuleBase::draw();
    }

}
#endif

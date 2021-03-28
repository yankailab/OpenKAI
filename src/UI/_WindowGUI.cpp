/*
 *  Created on: Mar 26, 2021
 *      Author: yankai
 */
#ifdef USE_GUI
#include "_WindowGUI.h"
#include "../Script/Kiss.h"

namespace kai
{

    _WindowGUI::_WindowGUI()
    {
    }

    _WindowGUI::~_WindowGUI()
    {
    }

    bool _WindowGUI::init(void *pKiss)
    {
        IF_F(!this->_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        return true;
    }

    bool _WindowGUI::setup(void)
    {
        return true;
    }

    int _WindowGUI::check(void)
    {
        return this->_ModuleBase::check();
    }

    bool _WindowGUI::start ( void )
    {
        NULL_F ( m_pT );
        return m_pT->start ( getUpdate, this );
    }

    void _WindowGUI::update ( void )
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

    bool _WindowGUI::updateGUI ( void )
    {
        return true;
    }

    // void _WindowGUI::CbRecvData ( LivoxEthPacket* pData, void* pLivox )
    // {
    // }

    void _WindowGUI::draw(void)
    {
        this->_ModuleBase::draw();
    }

}
#endif

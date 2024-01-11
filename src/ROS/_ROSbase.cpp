/*
 * _ROSbase.cpp
 *
 *  Created on: Jan 7, 2024
 *      Author: yankai
 */

#include "_ROSbase.h"

namespace kai
{

    _ROSbase::_ROSbase()
    {
        m_pTPP = NULL;

    }

    _ROSbase::~_ROSbase()
    {
        DEL(m_pTPP);
    }

    bool _ROSbase::init(void *pKiss)
    {
        IF_F(!_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

//		pK->v("URI", &m_devURI);

        return true;
    }

    bool _ROSbase::link(void)
    {
        IF_F(!this->_ModuleBase::link());
        Kiss *pK = (Kiss *)m_pKiss;

        string n;

        n = "";
        pK->v("_SHMrgb", &n);
        m_psmRGB = (_SharedMem *)(pK->getInst(n));

        return true;
    }

    bool _ROSbase::open(void)
    {
        return false;
    }

    void _ROSbase::close(void)
    {
    }

    int _ROSbase::check(void)
    {
//        NULL__(m_pT, -1);

        return _ModuleBase::check();
    }

	void _ROSbase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
//		pC->addMsg("nState: " + i2str(m_vStates.size()), 0);
	}

}

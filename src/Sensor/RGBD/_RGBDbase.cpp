/*
 * _RGBDbase.cpp
 *
 *  Created on: Jan 2, 2024
 *      Author: yankai
 */

#include "_RGBDbase.h"

namespace kai
{

    _RGBDbase::_RGBDbase()
    {
		m_deviceURI = "";
        m_bOpen = false;
        m_vSize.set(1600, 1200);

        m_bRGB = true;
        m_bDepth = true;
        m_btRGB = false;
        m_btDepth = false;
        m_bIR = false;
		m_tWait = 80;

        m_psmRGB = NULL;
        m_psmDepth = NULL;
        m_psmTransformedRGB = NULL;
        m_psmTransformedDepth = NULL;
        m_psmIR = NULL;

        m_pTPP = NULL;
    }

    _RGBDbase::~_RGBDbase()
    {
        DEL(m_pTPP);
    }

    bool _RGBDbase::init(void *pKiss)
    {
        IF_F(!_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

		pK->v("URI", &m_deviceURI);
        pK->v("vSize", &m_vSize);
		pK->v("tWait", &m_tWait);

        pK->v("bRGB", &m_bRGB);
        pK->v("bDepth", &m_bDepth);
        pK->v("btRGB", &m_btRGB);
        pK->v("btDepth", &m_btDepth);
        pK->v("bIR", &m_bIR);
        pK->v("fConfidenceThreshold", &m_fConfidenceThreshold);

        return true;
    }

    bool _RGBDbase::link(void)
    {
        IF_F(!this->_ModuleBase::link());
        Kiss *pK = (Kiss *)m_pKiss;

        string n;

        n = "";
        pK->v("_SHMrgb", &n);
        m_psmRGB = (_SharedMem *)(pK->getInst(n));

        n = "";
        pK->v("_SHMtransformedRGB", &n);
        m_psmTransformedRGB = (_SharedMem *)(pK->getInst(n));

        n = "";
        pK->v("_SHMdepth", &n);
        m_psmDepth = (_SharedMem *)(pK->getInst(n));

        n = "";
        pK->v("_SHMtransformedDepth", &n);
        m_psmTransformedDepth = (_SharedMem *)(pK->getInst(n));

        n = "";
        pK->v("_SHMir", &n);
        m_psmIR = (_SharedMem *)(pK->getInst(n));

        return true;
    }

    bool _RGBDbase::open(void)
    {
        return false;
    }

    void _RGBDbase::close(void)
    {
    }

    int _RGBDbase::check(void)
    {
        NULL__(m_pT, -1);

        if (m_bRGB)
        {
            NULL__(m_psmRGB, -1);
            IF__(!m_psmRGB->open(), -1);
        }

        if (m_bDepth)
        {
            NULL__(m_psmDepth, -1);
            IF__(!m_psmDepth->open(), -1);
        }

        if (m_btRGB)
        {
            NULL__(m_psmTransformedRGB, -1);
            IF__(!m_psmTransformedRGB->open(), -1);
        }

        if (m_btDepth)
        {
            NULL__(m_psmTransformedDepth, -1);
            IF__(!m_psmTransformedDepth->open(), -1);
        }

        if (m_bIR)
        {
            NULL__(m_psmIR, -1);
            IF__(!m_psmIR->open(), -1);
        }

        return _ModuleBase::check();
    }

	void _RGBDbase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
//		pC->addMsg("nState: " + i2str(m_vStates.size()), 0);
	}

}

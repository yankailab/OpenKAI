#include "_AProver_WBnav.h"

namespace kai
{

    _AProver_WBnav::_AProver_WBnav()
    {
        m_pAP = NULL;
        m_pD = NULL;
        m_pUside = NULL;
        m_pPIDside = NULL;

        m_errSide = 0.0;
        m_targetSide = 0.4;
        m_vSide.set(0.2, 0.8);

        m_pUdivider = NULL;
        m_bDivider = false;

        m_pUtag = NULL;
        m_iTag = -1;
        m_vTagY.set(0.2, 0.8);
        m_vTagX.set(0.2, 0.8);

        m_nSpd = 0.0;
        m_nStr = 0.0;
    }

    _AProver_WBnav::~_AProver_WBnav()
    {
    }

    bool _AProver_WBnav::init(void *pKiss)
    {
        IF_F(!this->_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;
    	

        pK->v("targetSide", &m_targetSide);
        pK->v("nSpd", &m_nSpd);
        pK->v("nStr", &m_nStr);
        pK->v("vTagY", &m_vTagY);
        pK->v("vTagX", &m_vTagX);
        pK->v("vSide", &m_vSide);

        IF_F(!m_pSC);
        IF_F(!m_iState.assign(m_pSC));

        string n;

        n = "";
        pK->v("_AP_base", &n);
        m_pAP = (_AP_base *)(pK->getInst(n));
        NULL_Fl(m_pAP, n + ": not found");

        n = "";
        pK->v("Drive", &n);
        m_pD = (_Drive *)(pK->getInst(n));
        IF_Fl(!m_pD, n + ": not found");

        n = "";
        pK->v("_UniverseSide", &n);
        m_pUside = (_Universe *)(pK->getInst(n));
        NULL_Fl(m_pUside, n + ": not found");

        n = "";
        pK->v("PIDside", &n);
        m_pPIDside = (PID *)(pK->getInst(n));
        NULL_Fl(m_pPIDside, n + " not found");

        n = "";
        pK->v("_UniverseDivider", &n);
        m_pUdivider = (_Universe *)(pK->getInst(n));
        NULL_Fl(m_pUdivider, n + ": not found");

        n = "";
        pK->v("_UniverseTag", &n);
        m_pUtag = (_Universe *)(pK->getInst(n));
        NULL_Fl(m_pUtag, n + ": not found");

        return true;
    }

    bool _AProver_WBnav::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    int _AProver_WBnav::check(void)
    {
        NULL__(m_pAP, -1);
        NULL__(m_pD, -1);
        NULL__(m_pUside, -1);
        NULL__(m_pPIDside, -1);
        NULL__(m_pUdivider, -1);
        NULL__(m_pUtag, -1);

        return this->_ModuleBase::check();
    }

    void _AProver_WBnav::update(void)
    {
        while (m_pT->bThread())
        {
            m_pT->autoFPSfrom();
            this->_ModuleBase::update();

            updateFollow();

            m_pT->autoFPSto();
        }
    }

    void _AProver_WBnav::updateFollow(void)
    {
        IF_(check() < 0);
        IF_(!bActive());

        float dir = m_pD->getDirection(); //+/-1.0
        float nSpd = m_nSpd;

        _Object *pO;

        // follow the left side border of the multi-sheet
        pO = m_pUside->get(0);
        float errSide = 0.0;
        if (pO)
        {
            if (m_vSide.bInside(pO->getY()))
                errSide = dir * (pO->getY() - m_targetSide);
        }
        m_errSide = errSide;
        m_nStr = dir * m_pPIDside->update(m_errSide, 0.0, m_pT->getTfrom());

        // make a stop at divider
        pO = m_pUdivider->get(0);
        if (pO)
        {
            if (!m_bDivider)
            {
                m_bDivider = true;
                nSpd = 0.0;
                m_nStr = 0.0;
                m_pSC->transit(m_iState.DIVIDER);
            }
        }
        else
        {
            m_bDivider = false;
        }

        // make a stop at tag
        pO = m_pUtag->get(0);
        if (pO)
        {
            int iTag = pO->getTopClass();
            if (m_iTag != iTag && m_vTagY.bInside(pO->getY()) && m_vTagX.bInside(pO->getX()))
            {
                nSpd = 0.0;
                m_nStr = 0.0;
                m_iTag = iTag;
                m_pSC->transit(m_iState.EXTRACT);
            }
        }

        m_pD->setSteering(m_nStr);
        m_pD->setSpeed(nSpd);
    }

    void _AProver_WBnav::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ModuleBase::console(pConsole);
        msgActive(pConsole);

        _Console *pC = (_Console *)pConsole;
        pC->addMsg("nSpd=" + f2str(m_nSpd) + ", nStr=" + f2str(m_nStr));
        pC->addMsg("errSide=" + f2str(m_errSide));
    }

}

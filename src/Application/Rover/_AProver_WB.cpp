#include "_AProver_WB.h"

namespace kai
{

    _AProver_WB::_AProver_WB()
    {
        m_pAP = NULL;
        m_pD = NULL;

        m_rcMode.set(0);
    }

    _AProver_WB::~_AProver_WB()
    {
    }

    bool _AProver_WB::init(void *pKiss)
    {
        IF_F(!this->_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;
    	pK->m_pInst = this;

        NULL_F(m_pSC);
        pK->v("iRCmodeChan", &m_rcMode.m_iChan);
        pK->a("vRCmodeDiv", &m_rcMode.m_vDiv);
        m_rcMode.update();

        m_iMode.MANUAL = m_pSC->getStateIdxByName("MANUAL");
        m_iMode.FORWARD = m_pSC->getStateIdxByName("FORWARD");
        m_iMode.BACKWARD = m_pSC->getStateIdxByName("BACKWARD");
        IF_F(!m_iMode.bValid());

        string n;
        n = "";
        pK->v("_AP_base", &n);
        m_pAP = (_AP_base *)(pK->getInst(n));
        IF_Fl(!m_pAP, n + ": not found");

        n = "";
        pK->v("Drive", &n);
        m_pD = (_Drive *)(pK->getInst(n));
        IF_Fl(!m_pD, n + ": not found");

        return true;
    }

    bool _AProver_WB::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    int _AProver_WB::check(void)
    {
        NULL__(m_pAP, -1);
        NULL__(m_pAP->m_pMav, -1);
        NULL__(m_pSC, -1);

        return this->_ModuleBase::check();
    }

    void _AProver_WB::update(void)
    {
        while (m_pT->bThread())
        {
            m_pT->autoFPSfrom();
            this->_ModuleBase::update();

            updateMode();

            m_pT->autoFPSto();
        }
    }

    void _AProver_WB::updateMode(void)
    {
        IF_(check() < 0);

        uint32_t apMode = m_pAP->getApMode();
        if (apMode != AP_ROVER_MANUAL)
        {
            m_pSC->transit(m_iMode.MANUAL);
            return;
        }

        float dir = 0.0;

        uint16_t pwmMode = m_pAP->m_pMav->m_rcChannels.getRC(m_rcMode.m_iChan);
        IF_(pwmMode == UINT16_MAX);
        m_rcMode.set(pwmMode);
        int iMode = m_rcMode.i();
        switch (iMode)
        {
        case 0:
            m_pSC->transit(m_iMode.BACKWARD);
            dir = -1.0;
            break;
        case 1:
            m_pSC->transit(m_iMode.MANUAL);
            break;
        case 2:
            m_pSC->transit(m_iMode.FORWARD);
            dir = 1.0;
            break;
        default:
            m_pSC->transit(m_iMode.MANUAL);
            break;
        }

        m_pD->setDirection(dir);
    }

    void _AProver_WB::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ModuleBase::console(pConsole);
        msgActive(pConsole);

        ((_Console *)pConsole)->addMsg("rcMode pwm=" + i2str(m_rcMode.m_raw) + ", i=" + i2str(m_rcMode.i()));
    }

}

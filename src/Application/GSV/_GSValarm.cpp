#include "_GSValarm.h"

namespace kai
{

    _GSValarm::_GSValarm()
    {
        m_bAlarm = false;
        m_bEnable = true;
        m_nObj = 0;
        m_pDio = NULL;
        m_iDout = 0;
        m_iDin = 1;

        m_pS = NULL;
        m_iSaxis = 0;
        m_spON = 1600;
        m_spOFF = 600;
    }

    _GSValarm::~_GSValarm()
    {
    }

    bool _GSValarm::init(void *pKiss)
    {
        IF_F(!this->_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("iDout", &m_iDout);
        pK->v("iDin", &m_iDin);
        pK->v("iSaxis", &m_iSaxis);
        pK->v("spON", &m_spON);
        pK->v("spOFF", &m_spOFF);

        return true;
    }

    bool _GSValarm::link(void)
    {
        IF_F(!this->_ModuleBase::link());
        Kiss *pK = (Kiss *)m_pKiss;

        string n;

        n = "";
        pK->v("_ADIObase", &n);
        m_pDio = (_ADIObase *)(pK->getInst(n));

        n = "";
        pK->v("_Feetech", &n);
        m_pS = (_Feetech *)(pK->getInst(n));

        vector<string> vN;
        pK->a("_Universe", &vN);
        for (string u : vN)
        {
            _Universe *pU = (_Universe *)(pK->getInst(u));
            IF_CONT(!pU);

            m_vpU.push_back(pU);
        }

        return true;
    }

    bool _GSValarm::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    int _GSValarm::check(void)
    {
        IF__(m_vpU.empty(), -1);

        return this->_ModuleBase::check();
    }

    void _GSValarm::update(void)
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPSfrom();

            updateGSV();

            m_pT->autoFPSto();
        }
    }

    void _GSValarm::updateGSV(void)
    {
        IF_(check() < 0);

        int n = 0;
        for (_Universe *pU : m_vpU)
        {
            int i = 0;
            _Object *pO = NULL;
            while ((pO = pU->get(i++)))
            {
                n++;
            }
        }
        m_nObj = n;

        if (m_pDio)
            m_bEnable = m_pDio->readD(m_iDin);

        if (m_nObj > 0 && m_bEnable)
            m_bAlarm = true;
        else
            m_bAlarm = false;

        if (m_bAlarm)
        {
            if (m_pDio)
                m_pDio->writeD(m_iDout, true);

            if (m_pS)
                m_pS->setPtarget(m_iSaxis, m_spON);
        }
        else
        {
            if (m_pDio)
                m_pDio->writeD(m_iDout, false);

            if (m_pS)
                m_pS->setPtarget(m_iSaxis, m_spOFF);
        }
    }

    void _GSValarm::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ModuleBase::console(pConsole);

        _Console *pC = (_Console *)pConsole;
        if (m_bEnable)
        {
            if (m_bAlarm)
                pC->addMsg("Enabled: ALARM!");
            else
                pC->addMsg("Enabled: SAFE");
        }
        else
        {
            pC->addMsg("Disabled");
        }

    }

}

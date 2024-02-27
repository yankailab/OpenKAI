#include "_GSVio.h"

namespace kai
{

    _GSVio::_GSVio()
    {
        m_pDio = NULL;
        m_iDin = 1;
        m_iDout = 0;
        m_iDout2 = 1;

        m_bAlarm = false;
        m_bEnable = true;
    }

    _GSVio::~_GSVio()
    {
    }

    bool _GSVio::init(void *pKiss)
    {
        IF_F(!this->_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;
		

        pK->v("iDin", &m_iDin);
        pK->v("iDout", &m_iDout);
        pK->v("iDout2", &m_iDout2);

        return true;
    }

    bool _GSVio::link(void)
    {
        IF_F(!this->_ModuleBase::link());
        Kiss *pK = (Kiss *)m_pKiss;

        string n;
        n = "";
        pK->v("_ADIObase", &n);
        m_pDio = (_ADIObase *)(pK->getInst(n));

        return true;
    }

    bool _GSVio::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    int _GSVio::check(void)
    {
        NULL__(m_pDio, -1);

        return this->_ModuleBase::check();
    }

    void _GSVio::update(void)
    {
        while (m_pT->bThread())
        {
            m_pT->autoFPSfrom();

            updateAlarm();

            m_pT->autoFPSto();
        }
    }

    void _GSVio::updateAlarm(void)
    {
        IF_(check() < 0);

        m_bEnable = m_pDio->readD(m_iDin);

        m_pDio->writeD(m_iDout, m_bAlarm);
        m_pDio->writeD(m_iDout2, m_bAlarm);
    }

    void _GSVio::setAlarm(bool bAlarm)
    {
        m_bAlarm = bAlarm;
    }

    void _GSVio::console(void *pConsole)
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

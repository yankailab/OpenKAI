#include "_GSVio.h"

namespace kai
{

    _GSVio::_GSVio()
    {
        m_pDio = NULL;

        m_iDinOn = 0;
        m_iDinOff = 1;

        m_iDoutLED = 0;
        m_iDoutNO = 1;
        m_iDoutNC = 2;

        m_bAlarm = false;
        m_bEnable = true;
    }

    _GSVio::~_GSVio()
    {
    }

    int _GSVio::init(void *pKiss)
    {
        CHECK_(this->_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("iDinOn", &m_iDinOn);
        pK->v("iDinOff", &m_iDinOff);

        pK->v("iDoutLED", &m_iDoutLED);
        pK->v("iDoutNO", &m_iDoutNO);
        pK->v("iDoutNC", &m_iDoutNC);

        return OK_OK;
    }

    int _GSVio::link(void)
    {
        CHECK_(this->_ModuleBase::link());
        Kiss *pK = (Kiss *)m_pKiss;

        string n;
        n = "";
        pK->v("_ADIObase", &n);
        m_pDio = (_ADIObase *)(pK->findModule(n));

        return OK_OK;
    }

    int _GSVio::start(void)
    {
        NULL__(m_pT, OK_ERR_NULLPTR);
        return m_pT->start(getUpdate, this);
    }

    int _GSVio::check(void)
    {
        NULL__(m_pDio, OK_ERR_NULLPTR);

        return this->_ModuleBase::check();
    }

    void _GSVio::update(void)
    {
        while (m_pT->bAlive())
        {
            m_pT->autoFPSfrom();

            updateAlarm();

            m_pT->autoFPSto();
        }
    }

    void _GSVio::updateAlarm(void)
    {
        IF_(check() != OK_OK);

        bool bOn = m_pDio->readD(m_iDinOn);
        bool bOff = m_pDio->readD(m_iDinOff);

        if (bOn)
            setEnable(true);
        else if (bOff)
            setEnable(false);

        m_pDio->writeD(m_iDoutLED, m_bEnable);
        m_pDio->writeD(m_iDoutNO, m_bAlarm & m_bEnable);
        m_pDio->writeD(m_iDoutNC, m_bAlarm & m_bEnable);
    }

    void _GSVio::setAlarm(bool bAlarm)
    {
        m_bAlarm = bAlarm | m_bAlarm;
    }

    void _GSVio::setEnable(bool bEnable)
    {
        m_bEnable = bEnable;

//        if(!m_bEnable)
        m_bAlarm = false;
        // reset the detection
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

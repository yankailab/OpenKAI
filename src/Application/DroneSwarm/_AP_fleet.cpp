#include "_AP_fleet.h"

namespace kai
{

    _AP_fleet::_AP_fleet()
    {
        m_pAP = NULL;
    }

    _AP_fleet::~_AP_fleet()
    {
    }

    bool _AP_fleet::init(void *pKiss)
    {
        IF_F(!this->_StateBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        return true;
    }

	bool _AP_fleet::link(void)
	{
		IF_F(!this->_StateBase::link());

		Kiss *pK = (Kiss *)m_pKiss;

        string n;
        n = "";
        pK->v("_AP_base", &n);
        m_pAP = (_AP_base *)(pK->getInst(n));
        IF_Fl(!m_pAP, n + ": not found");

		return true;
	}

    bool _AP_fleet::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    int _AP_fleet::check(void)
    {
        NULL__(m_pAP, -1);
        NULL__(m_pAP->m_pMav, -1);

        return this->_StateBase::check();
    }

    void _AP_fleet::update(void)
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPSfrom();

            updateFleet();

            m_pT->autoFPSto();
        }
    }

    void _AP_fleet::updateFleet(void)
    {
        IF_(check() < 0);

        int apMode = m_pAP->getApMode();
        bool bApArmed = m_pAP->bApArmed();
        float alt = m_pAP->getGlobalPos().w; //relative altitude

        IF_(!bApArmed);        
        IF_(apMode != AP_COPTER_GUIDED);

        m_pAP->setApMode(AP_COPTER_LAND);
    }

    void _AP_fleet::console(void *pConsole)
    {
        this->_StateBase::console(pConsole);
    }

}

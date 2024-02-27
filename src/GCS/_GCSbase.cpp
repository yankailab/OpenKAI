#include "_GCSbase.h"

namespace kai
{

    _GCSbase::_GCSbase()
    {
        m_pSC = NULL;
        m_gcsID = -1;
    }

    _GCSbase::~_GCSbase()
    {
    }

    bool _GCSbase::init(void *pKiss)
    {
        IF_F(!this->_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("gcsID", &m_gcsID);

        return true;
    }

	bool _GCSbase::link(void)
	{
		IF_F(!this->_ModuleBase::link());
		Kiss *pK = (Kiss *)m_pKiss;
    	

        string n;
        n = "";
        pK->v("_StateControl", &n);
        m_pSC = (_StateControl *)(pK->getInst(n));
        IF_Fl(!m_pSC, n + ": not found");

        IF_F(!m_state.assign(m_pSC));

		return true;
	}

    int _GCSbase::check(void)
    {
        NULL__(m_pSC, -1);

        return this->_ModuleBase::check();
    }

    void _GCSbase::updateGCS(void)
    {
        IF_(check() < 0);

        m_state.update(m_pSC);
    }

    int _GCSbase::getID(void)
    {
        return m_gcsID;
    }

    GCS_STATE *_GCSbase::getState(void)
    {
        return &m_state;
    }

    void _GCSbase::status(int vID, const string &stat)
    {
        IF_(check() < 0);

        if (stat == "standby")
            m_pSC->transit(m_state.STANDBY);
        if (stat == "airborne")
            m_pSC->transit(m_state.AIRBORNE);
        if (stat == "landed")
            m_pSC->transit(m_state.LANDED);
    }

    bool _GCSbase::takeoffRequest(int vID)
    {
        IF_F(check() < 0);
        IF_T(m_state.bTAKEOFF_READY());

        m_pSC->transit(m_state.TAKEOFF_REQUEST);
        return false;
    }

    bool _GCSbase::landingRequest(int vID)
    {
        IF_F(check() < 0);
        IF_T(m_state.bLANDING());

        m_pSC->transit(m_state.LANDING_REQUEST);
        return false;
    }

}

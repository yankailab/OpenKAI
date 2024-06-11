#include "_DroneBox.h"

namespace kai
{

    _DroneBox::_DroneBox()
    {
        m_pSC = NULL;
        m_pMB = NULL;
        m_iSlave = 1;
        m_lastCMD = dbx_unknown;

        m_ID = -1;
        m_vPos.clear();
    }

    _DroneBox::~_DroneBox()
    {
    }

    bool _DroneBox::init(void *pKiss)
    {
        IF_F(!this->_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;
		
        pK->v("ID", &m_ID);
        pK->v("vPos", &m_vPos);
        pK->v("iSlave", &m_iSlave);

        return true;
    }

	bool _DroneBox::link(void)
	{
		IF_F(!this->_ModuleBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

        string n;
        n = "";
        pK->v("_StateControl", &n);
        m_pSC = (_StateControl *)(pK->getInst(n));
        IF_Fl(!m_pSC, n + ": not found");

        n = "";
        pK->v("_Modbus", &n);
        m_pMB = (_Modbus *)(pK->getInst(n));
        IF_Fl(!m_pMB, n + ": not found");

		return true;
	}

    bool _DroneBox::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    int _DroneBox::check(void)
    {
        NULL__(m_pMB, -1);
        IF__(!m_pMB->bOpen(), -1);

        return this->_ModuleBase::check();
    }

    void _DroneBox::update(void)
    {
        while (m_pT->bAlive())
        {
            m_pT->autoFPSfrom();

            updateDroneBox();

            m_pT->autoFPSto();
        }
    }

    void _DroneBox::updateDroneBox(void)
    {
        IF_(check() < 0);

        string state = getState();

        if (state == "STANDBY")
        {
//            boxRecover();
            return;
        }

        //Takeoff procedure
        if (state == "TAKEOFF_REQUEST")
        {
            IF_(!boxTakeoffPrepare());

            if (bBoxTakeoffReady())
                m_pSC->transit("TAKEOFF_READY");

            return;
        }

        if (state == "TAKEOFF_READY")
        {
            return;
        }

        if (state == "AIRBORNE")
        {
            boxTakeoffComplete();
            return;
        }

        //Landing procedure
        if (state == "LANDING_REQUEST")
        {
            IF_(!boxLandingPrepare());

            if (bBoxLandingReady())
                m_pSC->transit("LANDING");

            return;
        }

        if (state == "LANDING")
        {
            return;
        }

        if (state == "TOUCHDOWN")
        {
            return;
        }

        if (state == "LANDED")
        {
            IF_(!boxLandingComplete());

            m_pSC->transit("STANDBY");
            return;
        }
    }

	string _DroneBox::getState(void)
    {
        return m_pSC->getCurrentStateName();
    }

	void _DroneBox::setState(const string& s)
    {
        return m_pSC->transit(s);
    }

    bool _DroneBox::takeoffRequest(void)
    {
        IF_F(check() < 0);

        IF_T(getState() == "TAKEOFF_READY");
        m_pSC->transit("TAKEOFF_REQUEST");
        return false;
    }

    bool _DroneBox::landingRequest(void)
    {
        IF_F(check() < 0);

        IF_T(getState() == "LANDING");
        m_pSC->transit("LANDING_REQUEST");
        return false;
    }

    bool _DroneBox::boxTakeoffPrepare(void)
    {
        IF_F(check() < 0);
        IF_T(m_lastCMD == dbx_takeoffPrepare);
        IF_T(m_lastCMD == dbx_bTakeoffReady);

        //01 06 00 03 00 01 B8 0A
        int r = m_pMB->writeRegister(m_iSlave, 3, 1);
        LOG_I("boxTakeoffPrepare: " + i2str(r));
        IF_F(r <= 0);

        m_lastCMD = dbx_takeoffPrepare;
        return true;
    }

    bool _DroneBox::bBoxTakeoffReady(void)
    {
        IF_F(check() < 0);

        //01 03 00 04 00 01 C5 CB
        uint16_t b = 0;
        int r = m_pMB->readRegisters(m_iSlave, 4, 1, &b);
        LOG_I("bBoxTakeoffReady: " + i2str(r) + ", " + i2str(b));
        IF_F(r != 1);

        m_lastCMD = dbx_bTakeoffReady;
        return (b == 1) ? true : false;
    }

    bool _DroneBox::boxTakeoffComplete(void)
    {
        IF_F(check() < 0);
        IF_T(m_lastCMD == dbx_takeoffComplete);

        //01 06 00 05 00 01 58 0B
        int r = m_pMB->writeRegister(m_iSlave, 5, 1);
        LOG_I("boxTakeoffComplete: " + i2str(r));
        IF_F(r <= 0);

        m_lastCMD = dbx_takeoffComplete;
        return true;
    }

    bool _DroneBox::boxLandingPrepare(void)
    {
        IF_F(check() < 0);
        IF_T(m_lastCMD == dbx_landingPrepare);
        IF_T(m_lastCMD == dbx_bLandingReady);

        //01 06 00 00 00 01 48 0A
        int r = m_pMB->writeRegister(m_iSlave, 0, 1);
        LOG_I("boxLandingPrepare: " + i2str(r));
        IF_F(r <= 0);

        m_lastCMD = dbx_landingPrepare;
        return true;
    }

    bool _DroneBox::bBoxLandingReady(void)
    {
        IF_F(check() < 0);

        //01 03 00 01 00 01 D5 CA
        uint16_t b = 0;
        int r = m_pMB->readRegisters(m_iSlave, 1, 1, &b);
        LOG_I("bBoxLandingReady: " + i2str(r) + ", " + i2str(b));
        IF_F(r != 1);

        m_lastCMD = dbx_bLandingReady;
        return (b == 1) ? true : false;
    }

    bool _DroneBox::boxLandingComplete(void)
    {
        IF_F(check() < 0);
        IF_F(m_lastCMD == dbx_landingComplete);

        //01 06 00 02 00 01 E9 CA
        int r = m_pMB->writeRegister(m_iSlave, 2, 1);
        LOG_I("boxLandingComplete: " + i2str(r));
        IF_F(r <= 0);

        m_lastCMD = dbx_landingComplete;
        return true;
    }

    bool _DroneBox::boxRecover(void)
    {
        IF_F(check() < 0);
        IF_T(m_lastCMD == dbx_boxRecover);

        //01 06 00 06 00 01 A8 0B
        int r = m_pMB->writeRegister(m_iSlave, 6, 1);
        LOG_I("boxRecover: " + i2str(r));
        IF_F(r <= 0);

        m_lastCMD = dbx_boxRecover;
        return true;
    }

    int _DroneBox::getID(void)
    {
        return m_ID;
    }

    vDouble2 _DroneBox::getPos(void)
    {
        return m_vPos;
    }

}

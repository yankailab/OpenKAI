#include "_Drive.h"

namespace kai
{

    _Drive::_Drive()
    {
        m_nSpd = 0.0;
        m_nDir = 1.0;
        m_nStr = 0.0;

        m_vSpdRange.set(-1.0, 1.0);
        m_vStrRange.set(-1.0, 1.0);
    }

    _Drive::~_Drive()
    {
    }

    bool _Drive::init(void *pKiss)
    {
        IF_F(!this->_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;
    	

        pK->v("nSpd", &m_nSpd);
        pK->v("nDir", &m_nDir);
        pK->v("nStr", &m_nStr);
        pK->v("vSpdRange", &m_vSpdRange);
        pK->v("vStrRange", &m_vStrRange);

        Kiss *pKM = pK->child("motor");
        NULL_Fl(pKM, "motor not found");

        int i = 0;
        while (1)
        {
            Kiss *pM = pKM->child(i++);
            if (pM->empty())
                break;

            DRIVE_MOTOR m;
            m.init();
            pM->v("kSpd", &m.m_kSpd);
            pM->v("kStr", &m.m_kStr);
            pM->v("iAxis", &m.m_iAxis);

            string n = "";
            pM->v("_ActuatorBase", &n);
            m.m_pActuator = (_ActuatorBase *)(pK->getInst(n));

            m_vM.push_back(m);
        }

        return true;
    }

    bool _Drive::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    int _Drive::check(void)
    {
        return this->_ModuleBase::check();
    }

    void _Drive::update(void)
    {
        while (m_pT->bAlive())
        {
            m_pT->autoFPSfrom();

            if (m_nSpd < 0.0)
                m_nStr *= -1.0;

            for (int i = 0; i < m_vM.size(); i++)
            {
                DRIVE_MOTOR *pM = &m_vM[i];
                pM->update(m_nSpd * m_nDir, m_nStr);
            }

            m_pT->autoFPSto();
        }
    }

    void _Drive::setSpeed(float nSpd)
    {
        m_nSpd = m_vSpdRange.constrain(nSpd);
    }

    void _Drive::setDirection(float nDir)
    {
        m_nDir = nDir;
    }

    void _Drive::setSteering(float nStr)
    {
        m_nStr = m_vStrRange.constrain(nStr);
    }

    float _Drive::getSpeed(void)
    {
        return m_nSpd;
    }

    float _Drive::getDirection(void)
    {
        return m_nDir;
    }

    float _Drive::getSteering(void)
    {
        return m_nStr;
    }

    float _Drive::getMotorSpeed(int iM)
    {
        IF__(iM >= m_vM.size(), 0.0);

        return m_vM[iM].m_spd;
    }

    void _Drive::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ModuleBase::console(pConsole);

        _Console *pC = (_Console *)pConsole;
        pC->addMsg("nSpd = " + f2str(m_nSpd) + ", nDir = " + f2str(m_nDir) + ", nStr = " + f2str(m_nStr));
        for (int i = 0; i < m_vM.size(); i++)
        {
            DRIVE_MOTOR *pM = &m_vM[i];
            pC->addMsg("iMotor" + i2str(i) + ": spd=" + f2str(pM->m_spd));
        }
    }

}

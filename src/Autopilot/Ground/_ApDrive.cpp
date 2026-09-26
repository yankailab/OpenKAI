#include "_ApDrive.h"

namespace kai
{

    _ApDrive::_ApDrive()
    {
    }

    _ApDrive::~_ApDrive()
    {
    }

    bool _ApDrive::loadConfig(void)
    {
        IF_F(!this->_AutopilotBase::loadConfig());
        const json &j = *m_pJ;

        jKv(j, "nSpd", m_nSpd);
        jKv(j, "nDir", m_nDir);
        jKv(j, "nStr", m_nStr);
        jKv<float>(j, "vSpdRange", m_vSpdRange);
        jKv<float>(j, "vStrRange", m_vStrRange);

        return true;
    }

    bool _ApDrive::link(void)
    {
        IF_F(!this->_AutopilotBase::link());
        const json &j = *m_pJ;

        const json *pJM = jK(j, "motors");
        IF__(!pJM || !pJM->is_object(), true);
        const json &jM = *pJM;

        for (auto it = jM.begin(); it != jM.end(); it++)
        {
            const json &Ji = it.value();
            IF_CONT(!Ji.is_object());

            DRIVE_MOTOR m;
            jKv(Ji, "kSpd", m.m_kSpd);
            jKv(Ji, "kStr", m.m_kStr);

            string n = "";
            jKv(Ji, "_ActuatorBase", n);
            m.m_pActuator = (_ActuatorBase *)(m_pM->findModule(n));

            m_vM.push_back(m);
        }

        return true;
    }

    bool _ApDrive::start(void)
    {
        NULL_F(m_pT);
        return m_pT->startThread(getUpdate, this);
    }

    bool _ApDrive::check(void)
    {
        return this->_AutopilotBase::check();
    }

    void _ApDrive::update(void)
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPS();

            if (m_nSpd < 0.0)
                m_nStr *= -1.0;

            for (DRIVE_MOTOR &m : m_vM)
            {
                m.update(m_nSpd * m_nDir, m_nStr);
            }
        }
    }

    void _ApDrive::setSpeed(float nSpd)
    {
        m_nSpd = std::clamp(nSpd, m_vSpdRange.x(), m_vSpdRange.y());
    }

    void _ApDrive::setDirection(float nDir)
    {
        m_nDir = nDir;
    }

    void _ApDrive::setSteering(float nStr)
    {
        m_nStr = std::clamp(nStr, m_vStrRange.x(), m_vStrRange.y());
    }

    float _ApDrive::getSpeed(void)
    {
        return m_nSpd;
    }

    float _ApDrive::getDirection(void)
    {
        return m_nDir;
    }

    float _ApDrive::getSteering(void)
    {
        return m_nStr;
    }

    float _ApDrive::getMotorSpeed(int iM)
    {
        IF__(iM < 0 || static_cast<size_t>(iM) >= m_vM.size(), 0.0);

        return m_vM[iM].m_spd;
    }

    void _ApDrive::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_AutopilotBase::console(pConsole);

        _Console *pC = (_Console *)pConsole;
        pC->addMsg("nSpd = " + f2str(m_nSpd) + ", nDir = " + f2str(m_nDir) + ", nStr = " + f2str(m_nStr));
        for (size_t i = 0; i < m_vM.size(); i++)
        {
            DRIVE_MOTOR *pM = &m_vM[i];
            pC->addMsg("iMotor" + i2str(static_cast<int>(i)) + ": spd=" + f2str(pM->m_spd));
        }
    }

}

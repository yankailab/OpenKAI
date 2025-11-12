#ifndef OpenKAI_src_Autopilot_Drive__Drive_H_
#define OpenKAI_src_Autopilot_Drive__Drive_H_

#include "../../Base/_ModuleBase.h"
#include "../../Actuator/_ActuatorBase.h"

namespace kai
{

    struct DRIVE_MOTOR
    {
        float m_kSpd; // velocity +/-
        float m_kStr;
        float m_spd;

        _ActuatorBase *m_pActuator;

        void init(void)
        {
            m_kSpd = 1.0;
            m_kStr = 1.0;
            m_spd = 0.0;
            m_pActuator = NULL;
        }

        void update(float nSpd, float nStr)
        {
            m_spd = nSpd * m_kSpd + nStr * m_kStr;

            NULL_(m_pActuator);
            m_pActuator->speed()->setTarget(m_spd);
        }
    };

    class _Drive : public _ModuleBase
    {
    public:
        _Drive();
        ~_Drive();

        virtual int init(void *pKiss);
        virtual int start(void);
        virtual int check(void);
        virtual void console(void *pConsole);

        virtual void setSpeed(float nSpd);
        virtual void setDirection(float nDir);
        virtual void setSteering(float nStr);

        virtual float getSpeed(void);
        virtual float getDirection(void);
        virtual float getSteering(void);

        virtual float getMotorSpeed(int iM);

    private:
        virtual void update(void);
        static void *getUpdate(void *This)
        {
            ((_Drive *)This)->update();
            return NULL;
        }

    protected:
        float m_nSpd;
        float m_nDir;
        float m_nStr;

        vFloat2 m_vSpdRange; // overall speed range
        vFloat2 m_vStrRange; // overall steering range

        vector<DRIVE_MOTOR> m_vM;
    };

}
#endif

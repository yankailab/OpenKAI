#ifndef OpenKAI_src_Autopilot_Ground__ApDrive_H_
#define OpenKAI_src_Autopilot_Ground__ApDrive_H_

#include "../_AutopilotBase.h"
#include "../../Actuator/_ActuatorBase.h"

namespace kai
{
    struct DRIVE_MOTOR
    {
        string m_configKey;
        float m_kSpd = 1.0; // velocity +/-
        float m_kStr = 1.0;
        float m_spd = 0.0;;
        _ActuatorBase* m_pActuator = nullptr;

        void update(float nSpd, float nStr)
        {
            m_spd = nSpd * m_kSpd + nStr * m_kStr;

            NULL_(m_pActuator);
            m_pActuator->speed()->setTarget(m_spd);
        }
    };

    class _ApDrive : public _AutopilotBase
    {
    public:
        _ApDrive();
        ~_ApDrive();

        virtual bool loadConfig(void) override;
        bool saveConfig(bool bExport) override;
		virtual bool link(void) override;
        virtual bool start(void);
        virtual bool check(void);
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
            ((_ApDrive *)This)->update();
            return NULL;
        }

    protected:
        float m_nSpd = 0.0;
        float m_nDir = 1.0;
        float m_nStr = 0.0;

        Vector2f m_vSpdRange = Vector2f(-1.0, 1.0);
        Vector2f m_vStrRange = Vector2f(-1.0, 1.0);

        vector<DRIVE_MOTOR> m_vM;
    };

}
#endif

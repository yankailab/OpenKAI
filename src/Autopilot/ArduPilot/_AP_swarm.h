#ifndef OpenKAI_src_Application_Autopilot_ArduPilot__AP_swarm_H_
#define OpenKAI_src_Application_Autopilot_ArduPilot__AP_swarm_H_

#include "_AP_base.h"

namespace kai
{

    struct AP_SWARM_STATE
    {
        int8_t m_iState;
        int8_t STANDBY;
        int8_t TAKEOFF;
        int8_t AIRBORNE;
        int8_t RTL;

        bool assign(_StateControl *pSC)
        {
            NULL_F(pSC);

            m_iState = -1;
            STANDBY = pSC->getStateIdxByName("STANDBY");
            TAKEOFF = pSC->getStateIdxByName("TAKEOFF");
            AIRBORNE = pSC->getStateIdxByName("AIRBORNE");
            RTL = pSC->getStateIdxByName("RTL");

            return bValid();
        }

        bool bValid(void)
        {
            IF_F(STANDBY < 0);
            IF_F(TAKEOFF < 0);
            IF_F(AIRBORNE < 0);
            IF_F(RTL < 0);

            return true;
        }

        void update(_StateControl *pSC)
        {
            NULL_(pSC);
            m_iState = pSC->getStateIdx();
        }

        bool bSTANDBY(void)
        {
            return (m_iState == STANDBY);
        }

        bool bTAKEOFF(void)
        {
            return (m_iState == TAKEOFF);
        }

        bool bAIRBORNE(void)
        {
            return (m_iState == AIRBORNE);
        }

        bool bRTL(void)
        {
            return (m_iState == RTL);
        }
    };

	class _AP_swarm : public _StateBase
	{
	public:
		_AP_swarm();
		~_AP_swarm();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);
		virtual void update(void);

	protected:
		virtual void updateSwarm(void);
		static void *getUpdate(void *This)
		{
			((_AP_swarm *)This)->update();
			return NULL;
		}

	public:
		_AP_base *m_pAP;
		AP_SWARM_STATE m_state;

		bool m_bAutoArm;
		float m_altAirborne;
		int m_dLanded;


	};

}
#endif

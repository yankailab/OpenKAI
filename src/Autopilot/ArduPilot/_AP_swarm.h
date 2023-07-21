#ifndef OpenKAI_src_Application_Autopilot_ArduPilot__AP_swarm_H_
#define OpenKAI_src_Application_Autopilot_ArduPilot__AP_swarm_H_

#include "_AP_base.h"
#include "../../Protocol/_Xbee.h"
#include "../../Swarm/_SwarmSearch.h"

namespace kai
{

    struct AP_SWARM_STATE
    {
        int8_t m_iState;
        int8_t STANDBY;
        int8_t TAKEOFF;
        int8_t AUTO;
        int8_t RTL;

        bool assign(_StateControl *pSC)
        {
            NULL_F(pSC);

            m_iState = -1;
            STANDBY = pSC->getStateIdxByName("STANDBY");
            TAKEOFF = pSC->getStateIdxByName("TAKEOFF");
            AUTO = pSC->getStateIdxByName("AUTO");
            RTL = pSC->getStateIdxByName("RTL");

            return bValid();
        }

        bool bValid(void)
        {
            IF_F(STANDBY < 0);
            IF_F(TAKEOFF < 0);
            IF_F(AUTO < 0);
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

        bool bAUTO(void)
        {
            return (m_iState == AUTO);
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

        // callback for Xbee recv
        void onRecvMsg(const XBframe_receivePacket& d);
    	static void sOnRecvMsg(void *pInst, XBframe_receivePacket d)
        {
            NULL_(pInst);
            ((_AP_swarm*)pInst)->onRecvMsg(d);
        }

		// swarm msg handlers
		void handleMsgSetState(const SWMSG_CMD_SETSTATE& m);

	protected:
		virtual void updateSwarm(void);
		virtual void updateState(void);
		static void *getUpdate(void *This)
		{
			((_AP_swarm *)This)->update();
			return NULL;
		}

	protected:
		_AP_base *m_pAP;
		AP_SWARM_STATE m_state;
        _Xbee* m_pXb;
        _SwarmSearch* m_pSwarm;

		bool m_bAutoArm;
		float m_altTakeoff;
	};

}
#endif

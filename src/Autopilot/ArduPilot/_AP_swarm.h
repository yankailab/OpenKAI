#ifndef OpenKAI_src_Application_Autopilot_ArduPilot__AP_swarm_H_
#define OpenKAI_src_Application_Autopilot_ArduPilot__AP_swarm_H_

#include "_AP_base.h"
#include "../../Protocol/_Xbee.h"
#include "../../Swarm/_SwarmSearch.h"

namespace kai
{
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
		// swarm nodes
		void send(void);
		void sendHB(void);
		void sendGCupdate(void);

		virtual void updateSwarm(void);
		virtual void updateState(void);
		static void *getUpdate(void *This)
		{
			((_AP_swarm *)This)->update();
			return NULL;
		}

	protected:
		_AP_base *m_pAP;
		SWARM_SEARCH_STATE_NODE m_state;
        _Xbee* m_pXb;
        _SwarmSearch* m_pSwarm;

		bool m_bAutoArm;
		float m_altTakeoff;
		uint64_t m_myID;

		INTERVAL_EVENT m_ieSendHB;
		INTERVAL_EVENT m_ieSendSetState;
		INTERVAL_EVENT m_ieSendGCupdate;
	};

}
#endif

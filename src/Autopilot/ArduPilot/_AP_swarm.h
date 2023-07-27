#ifndef OpenKAI_src_Application_Autopilot_ArduPilot__AP_swarm_H_
#define OpenKAI_src_Application_Autopilot_ArduPilot__AP_swarm_H_

#include "_AP_move.h"
#include "../../Protocol/_Xbee.h"
#include "../../Swarm/_SwarmSearch.h"
#include "../../Universe/_Universe.h"

namespace kai
{
	struct SWARM_DETECTION
	{
		uint16_t m_id;
		vDouble2 m_vPos;
	};

	class _AP_swarm : public _AP_move
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
		// gimbal
		virtual void gimbalDownward(void);
		virtual char calcCRC(const char *cmd, uint8_t len);

		// swarm nodes
		virtual void send(void);
		virtual void sendHB(void);
		virtual void sendGCupdate(void);
		virtual void sendDetectionHB(void);

		// search actions
		virtual bool findBeacon(void);
		virtual bool findVisual(void);
		virtual bool findVisualTarget(void);
		virtual void calcMove(void);

		SWARM_DETECTION* getDetByID(uint16_t id);
		SWARM_DETECTION* getDetByDist(vDouble2 vP, double r);

		// update routine
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
		_Universe *m_pU;
		_IOBase *m_pGio;

		bool m_bAutoArm;
		float m_altTakeoff;
		float m_altAuto;
		float m_altLand;
		uint16_t m_myID;

		vInt2 m_vTargetID;
		int m_iClass;
		vector<SWARM_DETECTION> m_vDetections;
		double m_dRdet;

		vDouble2 m_vWP;
		vDouble2 m_vWPd;
		vDouble2 m_vWPrange;
		INTERVAL_EVENT m_ieNextWP;

		INTERVAL_EVENT m_ieSendHB;
		INTERVAL_EVENT m_ieSendSetState;
		INTERVAL_EVENT m_ieSendGCupdate;
	};

}
#endif

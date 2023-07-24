#ifndef OpenKAI_src_App_SwarmSearch__SwarmSearchCtrl_H_
#define OpenKAI_src_App_SwarmSearch__SwarmSearchCtrl_H_

#include "../../State/_StateBase.h"
#include "../../Protocol/_Xbee.h"
#include "../../Swarm/_SwarmSearch.h"

namespace kai
{
	struct SWARM_SEARCH_CTRL_NODE
	{
		uint16_t m_id = 0;
		vDouble2 m_pos = {0,0};
		float	 m_alt = 0;
	};

	class _SwarmSearchCtrl : public _StateBase
	{
	public:
		_SwarmSearchCtrl();
		~_SwarmSearchCtrl();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

		// swarm nodes
		// callback for Xbee recv
		static void sOnRecvMsg(void *pInst, XBframe_receivePacket d)
		{
			NULL_(pInst);
			((_SwarmSearchCtrl *)pInst)->onRecvMsg(d);
		}
		void onRecvMsg(const XBframe_receivePacket &d);

		// UI
		bool setState(const string& state);

		bool updateGridCells();
		bool clearGridCells(void);
		void getGridCells();

	protected:
		// swarm nodes
		void send(void);
		void sendHB(void);
		void sendSetState(void);
		void sendGCupdate(void);
		void handleMsgSetState(const SWMSG_CMD_SETSTATE &m);

		// UI

	private:
		void update(void);
		static void *getUpdate(void *This)
		{
			((_SwarmSearchCtrl *)This)->update();
			return NULL;
		}

	public:
		_Xbee *m_pXb;
		_SwarmSearch *m_pSwarm;
		SWARM_SEARCH_STATE_NODE m_state;
		SWARM_SEARCH_CTRL_NODE m_node;

		INTERVAL_EVENT m_ieSendHB;
		INTERVAL_EVENT m_ieSendSetState;
		INTERVAL_EVENT m_ieSendGCupdate;
	};

}
#endif

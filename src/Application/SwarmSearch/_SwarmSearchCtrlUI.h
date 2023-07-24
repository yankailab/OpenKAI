#ifndef OpenKAI_src_App_SwarmSearch__SwarmSearchCtrlUI_H_
#define OpenKAI_src_App_SwarmSearch__SwarmSearchCtrlUI_H_

#include "../../Protocol/_JSONbase.h"
#include "_SwarmSearchCtrl.h"

namespace kai
{

	class _SwarmSearchCtrlUI : public _JSONbase
	{
	public:
		_SwarmSearchCtrlUI();
		~_SwarmSearchCtrlUI();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

	protected:
		virtual void send(void);
		virtual void sendHeartbeat(void);
		virtual void sendNodeUpdate(void);
		virtual void sendNodeClearAll(void);

		// msg handlers
		void handleMsg(string &str);
		void setState(picojson::object &o);

	private:
		void updateW(void);
		static void *getUpdateW(void *This)
		{
			((_SwarmSearchCtrlUI *)This)->updateW();
			return NULL;
		}

		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_SwarmSearchCtrlUI *)This)->updateR();
			return NULL;
		}

	public:
		_Thread *m_Tr;
		_SwarmSearchCtrl* m_pCtrl;
		_SwarmSearch *m_pSwarm;
		
		INTERVAL_EVENT m_ieSendNodeUpdate;
		INTERVAL_EVENT m_ieSendNodeClearAll;

	};

}
#endif

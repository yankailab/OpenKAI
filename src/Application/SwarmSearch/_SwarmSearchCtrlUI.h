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
		void send(void);
		virtual bool sendHeartbeat(void);

		// msg handlers
		void handleMsg(string &str);
		void heartbeat(picojson::object &o);
		void stat(picojson::object &o);
		void req(picojson::object &o);

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
		_SwarmSearchCtrl* m_pSSC;
	};

}
#endif

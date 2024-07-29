#ifndef OpenKAI_src_Application_DroneBox__DroneBoxJSON_H_
#define OpenKAI_src_Application_DroneBox__DroneBoxJSON_H_

#include "../../Protocol/_JSONbase.h"
#include "_DroneBox.h"

namespace kai
{

	class _DroneBoxJSON : public _JSONbase
	{
	public:
		_DroneBoxJSON();
		~_DroneBoxJSON();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

	protected:
		void send(void);
	    virtual void sendHeartbeat(void);
	    virtual void sendStat(void);

		//msg handlers
		void handleMsg(string &str);
		void heartbeat(picojson::object &o);
		void stat(picojson::object &o);
		void req(picojson::object &o);

	private:
		void updateW(void);
		static void *getUpdateW(void *This)
		{
			((_DroneBoxJSON *)This)->updateW();
			return NULL;
		}

		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_DroneBoxJSON *)This)->updateR();
			return NULL;
		}

	protected:
		_DroneBox *m_pDB;
	};

}
#endif

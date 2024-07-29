#ifndef OpenKAI_src_Application_DroneBox__AP_droneBoxJSON_H_
#define OpenKAI_src_Application_DroneBox__AP_droneBoxJSON_H_

#include "../../Protocol/_JSONbase.h"
#include "_AP_droneBox.h"

namespace kai
{

	class _AP_droneBoxJSON : public _JSONbase
	{
	public:
		_AP_droneBoxJSON();
		~_AP_droneBoxJSON();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

	protected:
		void send(void);

		//msg handlers
		void handleMsg(string &str);
		void heartbeat(picojson::object &o);
		void stat(picojson::object &o);
		void ack(picojson::object &o);

	private:
		void updateW(void);
		static void *getUpdateW(void *This)
		{
			((_AP_droneBoxJSON *)This)->updateW();
			return NULL;
		}

		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_AP_droneBoxJSON *)This)->updateR();
			return NULL;
		}

	public:
		_AP_droneBox *m_pAPdroneBox;

		int m_ID;
		vDouble2 m_vPos;
	};

}
#endif

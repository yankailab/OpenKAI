#ifndef OpenKAI_src_Autopilot_APmavlink__APmavlink_mav2json_H_
#define OpenKAI_src_Autopilot_APmavlink__APmavlink_mav2json_H_

#include "../../Protocol/_JSONbase.h"
#include "_APmavlink_base.h"

namespace kai
{
	class _APmavlink_mav2json : public _JSONbase
	{
	public:
		_APmavlink_mav2json();
		~_APmavlink_mav2json();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

	private:
		void send(void);
		void updateVehicle(void);
		void updateW(void);
		static void *getUpdateW(void *This)
		{
			((_APmavlink_mav2json *)This)->updateW();
			return NULL;
		}

		void handleMsg(string &str);
		void heartbeat(picojson::object &o);
//		void stat(picojson::object &o);
		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_APmavlink_mav2json *)This)->updateR();
			return NULL;
		}

	protected:
		_APmavlink_base *m_pAP;
	};

}
#endif

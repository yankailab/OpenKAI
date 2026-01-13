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

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);

	protected:
		virtual void send(void);
		virtual void handleJson(const string &str);

		virtual void handleHeartbeat(picojson::object &o);
		virtual void handleStat(picojson::object &o);

	private:
		void updateW(void);
		static void *getUpdateW(void *This)
		{
			((_APmavlink_mav2json *)This)->updateW();
			return NULL;
		}

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

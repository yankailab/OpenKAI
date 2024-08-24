#ifndef OpenKAI_src_Autopilot_APmavlink__APmavlink_json_H_
#define OpenKAI_src_Autopilot_APmavlink__APmavlink_json_H_

#include "../../Protocol/_JSONbase.h"
#include "_APmavlink_move.h"

namespace kai
{
	class _APmavlink_json : public _JSONbase
	{
	public:
		_APmavlink_json();
		~_APmavlink_json();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

	private:
		void send(void);
		void updateMission(void);
		void updateW(void);
		static void *getUpdateW(void *This)
		{
			((_APmavlink_json *)This)->updateW();
			return NULL;
		}

		// msg handlers
		void handleMsg(string &str);
		void heartbeat(picojson::object &o);
		void stat(picojson::object &o);
		void missionUpdate(picojson::object &o);
		void missionStart(picojson::object &o);
		void missionPause(picojson::object &o);
		void missionResume(picojson::object &o);
		void missionStop(picojson::object &o);
		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_APmavlink_json *)This)->updateR();
			return NULL;
		}

	protected:
		_APmavlink_base *m_pAP;
		_APmavlink_move *m_pAPmove;
		int m_iMission; // current mission index
		bool m_bMissionGoing;
		double m_dS;
		int m_nCmdSent; // temporal for reposition command
	};

}
#endif

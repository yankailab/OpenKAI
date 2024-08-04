#ifndef OpenKAI_src_Autopilot_MavAP__MavAP_mission_H_
#define OpenKAI_src_Autopilot_MavAP__MavAP_mission_H_

#include "../../Protocol/_JSONbase.h"
#include "_MavAP_move.h"

namespace kai
{
	struct AP_MISSION
	{
		int m_missionID;
		vDouble4 m_vP;	// lat, lng, alt, hdg
		vDouble4 m_vPlookAt;
		float m_spd;
		uint64_t m_tDelay;

		void clear(void)
		{
			m_missionID = -1;
			m_vP.clear();
			m_vPlookAt.clear();
			m_spd = 0.0;
			m_tDelay = 0;
		}
	};

	class _MavAP_mission : public _JSONbase
	{
	public:
		_MavAP_mission();
		~_MavAP_mission();

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
			((_MavAP_mission *)This)->updateW();
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
			((_MavAP_mission *)This)->updateR();
			return NULL;
		}

	protected:
		_MavAP_base *m_pAP;
		_MavAP_move *m_pAPmove;
		vector<AP_MISSION> m_vMission;
		int m_iMission; // current mission index
		bool m_bMissionGoing;
		double m_dS;
		int m_nCmdSent; // temporal for reposition command
	};

}
#endif

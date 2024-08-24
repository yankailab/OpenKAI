#ifndef OpenKAI_src_Autopilot_APmavlink__APmavlink_mission_H_
#define OpenKAI_src_Autopilot_APmavlink__APmavlink_mission_H_

#include "../../Protocol/_JSONbase.h"
#include "_APmavlink_move.h"

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

	enum AP_MISSION_STATE
	{
		apMission_none,
		apMission_uploading,
		apMission_downloading,

	};

	class _APmavlink_mission : public _ModuleBase
	{
	public:
		_APmavlink_mission();
		~_APmavlink_mission();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

		void clearMission(void);
		void downloadMission(void);
		void uploadMission(void);

	private:
		void send(void);
		void updateMission(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_APmavlink_mission *)This)->update();
			return NULL;
		}

	protected:
		// mavlink mission protocol
		void missionCount(void);
		void missionRequestList(void);
		void missionRequestInt(void);
		void missionItemInt(void);
		void missionAck(void);
		void missionCurrent(void);
		void missionSetCurrent(void);
		void statusText(void);
		void missionClearAll(void);
		void missionItemReached(void);

	protected:
		_APmavlink_base *m_pAP;
		vector<AP_MISSION> m_vMission;
		int m_mID; // current mission index

		AP_MISSION_STATE m_mState;

		
	};

}
#endif

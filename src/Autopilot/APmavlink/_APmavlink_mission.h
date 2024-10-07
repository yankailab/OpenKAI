#ifndef OpenKAI_src_Autopilot_APmavlink__APmavlink_mission_H_
#define OpenKAI_src_Autopilot_APmavlink__APmavlink_mission_H_

#include "../../Protocol/_JSONbase.h"
#include "../../Utility/utilEvent.h"
#include "_APmavlink_move.h"

namespace kai
{
	struct AP_MISSION
	{
		int m_missionID;
		uint8_t m_missionType;
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
		apMission_none = 0,

		apMission_UL_missionCount = 1,
		apMission_UL_missionRequestInt = 2,
		apMission_UL_missionAck = 3,

		apMission_DL_missionRequestList = 4,
		apMission_DL_missionCount = 5,
		apMission_DL_missionRequestInt = 6,
		apMission_DL_missionAck = 7,
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

		// Upload
        static void sCbMavRecvMissionRequestInt(void* pMsg, void *pInst)
        {
            NULL_(pInst);
            ((_APmavlink_mission *)pInst)->CbMavRecvMissionRequestInt(pMsg);
        }

        static void sCbMavRecvMissionAck(void* pMsg, void *pInst)
        {
            NULL_(pInst);
            ((_APmavlink_mission *)pInst)->CbMavRecvMissionAck(pMsg);
        }

		// Download
        static void sCbMavRecvMissionCount(void* pMsg, void *pInst)
        {
            NULL_(pInst);
            ((_APmavlink_mission *)pInst)->CbMavRecvMissionCount(pMsg);
        }

        static void sCbMavRecvMissionItemInt(void* pMsg, void *pInst)
        {
            NULL_(pInst);
            ((_APmavlink_mission *)pInst)->CbMavRecvMissionItemInt(pMsg);
        }

	private:
		void updateMission(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_APmavlink_mission *)This)->update();
			return NULL;
		}

	protected:
		// upload
		void sendMissionCount(void);
		void CbMavRecvMissionRequestInt(void* pMsg);
		void CbMavRecvMissionAck(void* pMsg);

		// download
		void sendMissionRequestList(void);
		void CbMavRecvMissionCount(void* pMsg);
		void sendMissionRequestInt(void);
		void CbMavRecvMissionItemInt(void* pMsg);
		void sendMissionAck(void);

		void checkTimeOut(void);
		void missionCurrent(void);
		void missionSetCurrent(void);
		void statusText(void);
		void missionClearAll(void);
		void missionItemReached(void);

	protected:
		_APmavlink_base *m_pAP;

		vector<AP_MISSION> m_vMissionUL;
		int m_iMissionUL;
		uint64_t m_tUpdatedUL;

		vector<AP_MISSION> m_vMissionDL;
		int m_nMissionDL;
		int m_iMissionDL;
		int m_mIdxDL; // current mission index
		uint64_t m_tUpdatedDL;


		AP_MISSION_STATE m_mState;
		TIME_OUT m_tOut;
		int m_tOutSec;

	};

}
#endif

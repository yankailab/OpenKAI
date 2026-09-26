#ifndef OpenKAI_src_Autopilot_FC_ArduPilot__APmav_mission_H_
#define OpenKAI_src_Autopilot_FC_ArduPilot__APmav_mission_H_

#include "../../../Protocol/_JSONbase.h"
#include "../../../Utility/utilEvent.h"
#include "_APmav_move.h"

namespace kai
{
	struct AP_MISSION
	{
		int m_missionID;
		uint8_t m_missionType;
		Vector4d m_vP = Vector4d::Zero(); // lat, lng, alt, hdg
		Vector4d m_vPlookAt = Vector4d::Zero();
		float m_spd;
		uint64_t m_tDelay;

		void clear(void)
		{
			m_missionID = -1;
			m_vP.setZero();
			m_vPlookAt.setZero();
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

	class _APmav_mission : public _ModuleBase
	{
	public:
		_APmav_mission();
		~_APmav_mission();

		virtual bool loadConfig(void) override;
		virtual bool link(void) override;
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);

		void clearMission(void);
		void downloadMission(void);
		void uploadMission(void);

		// Upload
		static void sCbMavRecvMissionRequestInt(void *pMsg, void *pInst)
		{
			NULL_(pInst);
			((_APmav_mission *)pInst)->CbMavRecvMissionRequestInt(pMsg);
		}

		static void sCbMavRecvMissionAck(void *pMsg, void *pInst)
		{
			NULL_(pInst);
			((_APmav_mission *)pInst)->CbMavRecvMissionAck(pMsg);
		}

		// Download
		static void sCbMavRecvMissionCount(void *pMsg, void *pInst)
		{
			NULL_(pInst);
			((_APmav_mission *)pInst)->CbMavRecvMissionCount(pMsg);
		}

		static void sCbMavRecvMissionItemInt(void *pMsg, void *pInst)
		{
			NULL_(pInst);
			((_APmav_mission *)pInst)->CbMavRecvMissionItemInt(pMsg);
		}

	private:
		void updateMission(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_APmav_mission *)This)->update();
			return NULL;
		}

	protected:
		// upload
		void sendMissionCount(void);
		void CbMavRecvMissionRequestInt(void *pMsg);
		void CbMavRecvMissionAck(void *pMsg);

		// download
		void sendMissionRequestList(void);
		void CbMavRecvMissionCount(void *pMsg);
		void sendMissionRequestInt(void);
		void CbMavRecvMissionItemInt(void *pMsg);
		void sendMissionAck(void);

		void checkTimeOut(void);
		void missionCurrent(void);
		void missionSetCurrent(void);
		void statusText(void);
		void missionClearAll(void);
		void missionItemReached(void);

	protected:
		_APmav_base *m_pAP = nullptr;

		vector<AP_MISSION> m_vMissionUL;
		int m_iMissionUL;
		uint64_t m_tUpdatedUL;

		vector<AP_MISSION> m_vMissionDL;
		int m_nMissionDL;
		int m_iMissionDL;
		int m_mIdxDL = -1; // current mission index
		uint64_t m_tUpdatedDL;

		AP_MISSION_STATE m_mState = apMission_none;
		TIME_OUT m_tOut;
		int m_tOutSec = 10;
	};

}
#endif

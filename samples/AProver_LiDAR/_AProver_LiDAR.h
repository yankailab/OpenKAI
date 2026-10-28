#ifndef OpenKAI_samples_AProver_LiDAR__AProver_LiDAR_H_
#define OpenKAI_samples_AProver_LiDAR__AProver_LiDAR_H_

#include <mutex>
#include <OpenKAI/Protocol/_JSONbase.h>
#include <OpenKAI/Navigation/_GeoFence.h>
#include <OpenKAI/Universe/Grid/_SelectableOctGrid.h>
#include <OpenKAI/Autopilot/FC/ArduPilot/_APmav_move.h>
#include <OpenKAI/Autopilot/FC/ArduPilot/_APmav_rover.h>

namespace kai
{
	enum AP_LIDAR_BTN
	{
		apLiDAR_btnNone = 0,
		apLiDAR_btnStop = 1,
		apLiDAR_btnLeft = 2,
		apLiDAR_btnRight = 3,
		apLiDAR_btnForward = 4,
		apLiDAR_btnBackward = 5,
	};

	enum AP_LIDAR_MODE
	{
		apLiDAR_modeStandby = 0,
		apLiDAR_modeManual = 1,
		apLiDAR_modeAuto = 2,
	};

	class _AProver_LiDAR : public _APmav_move
	{
	public:
		_AProver_LiDAR();
		~_AProver_LiDAR();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(InstanceMgr *pM) override;
		virtual bool start(void);
		void stop(void) override;
		virtual bool check(void);
		virtual void update(void);
		virtual void console(void *pConsole);
		virtual void console(const json &j, void *pJSONbase);

		virtual void setSteerSpeed(float steer, float spd);

	protected:
		virtual void onPause(void);
		virtual void updateButton(void);
		virtual bool updateCtrl(void);
		virtual void updateDrive(void);
		bool getNavigation(Vector4d &position);
		void resetAvoidance(void);
		void stopControl(bool cutterOff);
		void updateAuto(float heading, const _GeoFence_STATUS &fence);
		static void *getUpdate(void *This)
		{
			((_AProver_LiDAR *)This)->update();
			return NULL;
		}

	protected:
		float m_steer = 0.0; // normalized, [-1, 1]
		float m_speed = 0.0; // normalized, [-1, 1]
		float m_pwmM = 1500;
		float m_pwmD = 500;
		uint8_t m_iRCsteer = 1;
		uint8_t m_iRCthrottle = 3;

		AP_LIDAR_BTN m_btnPressed = apLiDAR_btnNone;
		uint64_t m_tLastBtn = 0;
		uint64_t m_tOutBtn = NSEC_SEC / 10; // time out in nsec for the m_btnPressed to be set to apLiDAR_btnNone, defaults to 100ms

		AP_LIDAR_MODE m_dMode = apLiDAR_modeStandby;
		int m_apModeMove = AP_ROVER_MANUAL;
		_GeoFence *m_pGfence = nullptr;
		_SelectableOctGrid *m_pOctGrid = nullptr;
		float m_octGridOccu = 1;
		float m_speedGo = 0.2;
		float m_steerTurn = 0.2;
		float m_bumperTurnDeg = 60; // signed heading offset on bumper release; positive = right
		float m_hdgToleranceDeg = 5;
		uint64_t m_tOutNav = 2 * NSEC_SEC;

		enum BUMPER_STATE { bumperClear, bumperReverse, bumperTurn };
		BUMPER_STATE m_bumperState = bumperClear;
		float m_bumperTargetHdg = 0;
		bool m_bFenceTurning = false;
		float m_fenceTurnDir = 1;
		bool m_bDirectControl = false;
		float m_directSteer = 0;
		float m_directSpeed = 0;
		// The web console and control worker share modes, commands and RC outputs.
		std::recursive_mutex m_ctrlMutex;

		bool m_bServoON = false;
		uint8_t m_iRCservo = 6;
		float m_pwmServoON = 2000;
		float m_pwmServoOFF = 1500;

		bool m_bEmergencyBtn = false;
		bool m_bBumper = false;

	};

}
#endif

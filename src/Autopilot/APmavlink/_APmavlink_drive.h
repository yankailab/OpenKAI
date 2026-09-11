#ifndef OpenKAI_src_Autopilot_APmavlink__APmavlink_drive_H_
#define OpenKAI_src_Autopilot_APmavlink__APmavlink_drive_H_

#include "../../Protocol/_JSONbase.h"
#include "../../Navigation/_GeoFence.h"
#include "../../3D/Grid/_OctreeGrid.h"
#include "_APmavlink_move.h"

namespace kai
{
	enum AP_DRIVE_BTN
	{
		apDrive_btnNone = 0,
		apDrive_btnStop = 1,
		apDrive_btnLeft = 2,
		apDrive_btnRight = 3,
		apDrive_btnForward = 4,
		apDrive_btnBackward = 5,
	};

	enum AP_DRIVE_MODE
	{
		apDrive_modeStandby = 0,
		apDrive_modeManual = 1,
		apDrive_modeAuto = 2,
	};

	class _APmavlink_drive : public _APmavlink_move
	{
	public:
		_APmavlink_drive();
		~_APmavlink_drive();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual bool check(void);
		virtual void update(void);
		virtual void console(void *pConsole);
		virtual void console(const json &j, void *pJSONbase);

		virtual void setSteerSpeed(float steer, float spd);

	protected:
		virtual void onPause(void);
		virtual bool updateCtrl(void);
		virtual void updateDrive(void);
		static void *getUpdate(void *This)
		{
			((_APmavlink_drive *)This)->update();
			return NULL;
		}

	protected:
		float m_steer = 0.0; // normalized, [-1, 1]
		float m_speed = 0.0; // normalized, [-1, 1]
		float m_pwmM = 1500;
		float m_pwmD = 500;
		uint8_t m_iRCsteer = 1;
		uint8_t m_iRCthrottle = 3;

		AP_DRIVE_BTN m_btnPressed;
		uint64_t m_tLastBtn;
		uint64_t m_tOutBtn; // time out in usec for the m_btnPressed to be set to apDrive_btnNone, defaults to 100ms

		AP_DRIVE_MODE m_dMode;
		int m_apModeMove = AP_ROVER_MANUAL;
		_GeoFence *m_pGfence;
		_OctreeGrid *m_pOctGrid;
		float m_octGridOccu;
		float m_speedGo;
		float m_steerTurn;
	};

}
#endif

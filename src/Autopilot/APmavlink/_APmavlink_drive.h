#ifndef OpenKAI_src_Autopilot_APmavlink__APmavlink_drive_H_
#define OpenKAI_src_Autopilot_APmavlink__APmavlink_drive_H_

#include "../../Protocol/_JSONbase.h"
#include "../../Navigation/_GeoFence.h"
#include "../../3D/Grid/_OctreeGrid.h"
#include "_APmavlink_base.h"


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

	class _APmavlink_drive : public _ModuleBase
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
		virtual void setYawMode(bool bRelative);

	protected:
		virtual void onPause(void);

		bool updateCtrl(void);
		bool updateDrive(void);
		static void *getUpdate(void *This)
		{
			((_APmavlink_drive *)This)->update();
			return NULL;
		}

	protected:
		_APmavlink_base *m_pAP;

		bool m_bSetYawSpeed;
		float m_yawMode;
		bool m_bRcChanOverride;

		float m_steer; // normalized
		float m_speed; // normalized
		float m_pwmM;
		float m_pwmD;
		uint16_t *m_pRcYaw;
		uint16_t *m_pRcThrottle;
		mavlink_rc_channels_override_t m_rcOverride;

		AP_DRIVE_BTN m_btnPressed;
		uint64_t m_tLastBtn;
		uint64_t m_tOutBtn; // time out in usec for the m_btnPressed to be set to apDrive_btnNone, defaults to 100ms


		_GeoFence* m_pGfence;
		_OctreeGrid* m_pOctGrid;
		float m_octGridOccu;
		float m_speedGo;
		float m_steerTurn;


	};

}
#endif

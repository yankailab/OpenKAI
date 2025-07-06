#ifndef OpenKAI_src_Autopilot_APmavlink__APmavlink_drive_H_
#define OpenKAI_src_Autopilot_APmavlink__APmavlink_drive_H_

#include "_APmavlink_base.h"
#include "../Drive/_Drive.h"

namespace kai
{

	class _APmavlink_drive : public _ModuleBase
	{
	public:
		_APmavlink_drive();
		~_APmavlink_drive();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);
		virtual void update(void);
		virtual void console(void *pConsole);

		virtual void setYawMode(bool bRelative);

	protected:
		virtual void onPause(void);

		bool updateDrive(void);
		static void *getUpdate(void *This)
		{
			((_APmavlink_drive *)This)->update();
			return NULL;
		}

	protected:
		_APmavlink_base *m_pAP;
//		_Drive *m_pD;

		bool m_bSetYawSpeed;
		float m_yawMode;
		bool m_bRcChanOverride;
		float m_pwmM;
		float m_pwmD;
		uint16_t *m_pRcYaw;
		uint16_t *m_pRcThrottle;
		mavlink_rc_channels_override_t m_rcOverride;
	};

}
#endif

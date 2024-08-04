#ifndef OpenKAI_src_Autopilot_MavAP__MavAProver_drive_H_
#define OpenKAI_src_Autopilot_MavAP__MavAProver_drive_H_

#include "_MavAP_base.h"
#include "../Drive/_Drive.h"

namespace kai
{

	class _MavAProver_drive : public _ModuleBase
	{
	public:
		_MavAProver_drive();
		~_MavAProver_drive();

		virtual int init(void *pKiss);
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
			((_MavAProver_drive *)This)->update();
			return NULL;
		}

	public:
		_MavAP_base *m_pAP;
		_Drive *m_pD;

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

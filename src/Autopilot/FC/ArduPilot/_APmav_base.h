#ifndef OpenKAI_src_Autopilot_FC_ArduPilot__APmav_base_H_
#define OpenKAI_src_Autopilot_FC_ArduPilot__APmav_base_H_

#include "../../../Protocol/_Mavlink.h"
#include "../../_AutopilotBase.h"

namespace kai
{
	struct AP_MOUNT
	{
		bool m_bEnable;
		mavlink_mount_control_t m_control;
		mavlink_mount_configure_t m_config;

		void init(void)
		{
			m_bEnable = false;
			m_control.input_a = 0; // pitch
			m_control.input_b = 0; // roll
			m_control.input_c = 0; // yaw
			m_control.save_position = 0;
			m_config.stab_pitch = 0;
			m_config.stab_roll = 0;
			m_config.stab_yaw = 0;
			m_config.mount_mode = 2;
		}
	};

	class _APmav_base : public _AutopilotBase
	{
	public:
		_APmav_base();
		~_APmav_base();

		virtual bool loadConfig(void) override;
		virtual bool link(void) override;
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);
		virtual void console(const json &j, void *pJSONbase);


		virtual AP_MODE getMode(void);
		virtual AP_ARM getArm(void);

		virtual void setCustomMode(int32_t m);
		virtual int32_t getCustomMode(void);

		// Mavlink
		virtual _Mavlink *getMavlink(void);




		// gimbal, payloads
		virtual void setMount(AP_MOUNT &mount);

		// status
		int getGPSfixType(void);
		int getGPShacc(void);


	protected:
		// update loop
		virtual void updateApMavSend(void);
		virtual void updateApMavRecv(void);

	private:
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_APmav_base *)This)->update();
			return NULL;
		}

	protected:
		_Mavlink *m_pMav = nullptr;

		INTERVAL_EVENT m_ieSendHB;
		INTERVAL_EVENT m_ieSendMsgInt;

		// status received from FC
		AP_MODE m_modeFC = apMode_unknown;
		AP_ARM m_armFC = apArm_unknown;

		int32_t m_customMode = -1;
		int32_t m_customModeFC = -1;




		int8_t m_gpsFixType = -1;
		int m_gpsHacc = INT32_MAX;
	};

}
#endif

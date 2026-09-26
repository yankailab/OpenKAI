#ifndef OpenKAI_src_Autopilot_FC_ArduPilot__APmav_rover_H_
#define OpenKAI_src_Autopilot_FC_ArduPilot__APmav_rover_H_

#include "_APmav_base.h"

#define AP_ROVER_N_CUSTOM_MODE 17

namespace kai
{

	enum AP_ROVER_CUSTOM_MODE
	{
		AP_ROVER_MANUAL = 0,
		AP_ROVER_ACRO = 1,
		AP_ROVER_STEERING = 3,
		AP_ROVER_HOLD = 4,
		AP_ROVER_LOITER = 5,
		AP_ROVER_FOLLOW = 6,
		AP_ROVER_SIMPLE = 7,
		AP_ROVER_AUTO = 10,
		AP_ROVER_RTL = 11,
		AP_ROVER_SMART_RTL = 12,
		AP_ROVER_GUIDED = 15,
		AP_ROVER_INITIALISING = 16
	};

	const string AP_ROVER_CUSTOM_MODE_NAME[28] =
		{"MANUAL", "ACRO", "?", "STEERING", "HOLD", "LOITER", "FOLLOW", "SIMPLE", "?",
		 "?", "AUTO", "RTL", "SMART_RTL", "?", "?", "GUIDED", "INITIALISING"};


	class _APmav_rover : public _APmav_base
	{
	public:
		_APmav_rover();
		~_APmav_rover();

		virtual bool loadConfig(void) override;
		virtual bool link(void) override;
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);
		virtual void console(const json &j, void *pJSONbase);

		// copter specific
		virtual string getModeName(void);

	protected:
		virtual void updateApMavSend(void);
		virtual void updateApMavRecv(void);

	private:
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_APmav_rover *)This)->update();
			return NULL;
		}

	protected:

	};

}
#endif

#ifndef OpenKAI_src_Autopilot_FC_ArduPilot__APmav_copter_H_
#define OpenKAI_src_Autopilot_FC_ArduPilot__APmav_copter_H_

#include "_APmav_base.h"

#define AP_COPTER_N_CUSTOM_MODE 27

namespace kai
{

	enum AP_COPTER_CUSTOM_MODE
	{
		AP_COPTER_STABILIZE = 0,	 // manual airframe angle with manual throttle
		AP_COPTER_ACRO = 1,			 // manual body-frame angular rate with manual throttle
		AP_COPTER_ALT_HOLD = 2,		 // manual airframe angle with automatic throttle
		AP_COPTER_AUTO = 3,			 // fully automatic waypoint control using mission commands
		AP_COPTER_GUIDED = 4,		 // fully automatic fly to coordinate or fly at velocity/direction using GCS immediate commands
		AP_COPTER_LOITER = 5,		 // automatic horizontal acceleration with automatic throttle
		AP_COPTER_RTL = 6,			 // automatic return to launching point
		AP_COPTER_CIRCLE = 7,		 // automatic circular flight with automatic throttle
		AP_COPTER_LAND = 9,			 // automatic landing with horizontal position control
		AP_COPTER_DRIFT = 11,		 // semi-automous position, yaw and throttle control
		AP_COPTER_SPORT = 13,		 // manual earth-frame angular rate control with manual throttle
		AP_COPTER_FLIP = 14,		 // automatically flip the vehicle on the roll axis
		AP_COPTER_AUTOTUNE = 15,	 // automatically tune the vehicle's roll and pitch gains
		AP_COPTER_POSHOLD = 16,		 // automatic position hold with manual override, with automatic throttle
		AP_COPTER_BRAKE = 17,		 // full-brake using inertial/GPS system, no pilot input
		AP_COPTER_THROW = 18,		 // throw to launch mode using inertial/GPS system, no pilot input
		AP_COPTER_AVOID_ADSB = 19,	 // automatic avoidance of obstacles in the macro scale - e.g. full-sized aircraft
		AP_COPTER_GUIDED_NOGPS = 20, // guided mode but only accepts attitude and altitude
		AP_COPTER_SMART_RTL = 21,	 // SMART_RTL returns to home by retracing its steps
		AP_COPTER_FLOWHOLD = 22,	 // FLOWHOLD holds position with optical flow without rangefinder
		AP_COPTER_FOLLOW = 23,		 // follow attempts to follow another vehicle or ground station
		AP_COPTER_ZIGZAG = 24,		 // ZIGZAG mode is able to fly in a zigzag manner with predefined point A and point B
		AP_COPTER_SYSTEMID = 25,	 // System ID mode produces automated system identification signals in the controllers
		AP_COPTER_AUTOROTATE = 26	 // Autonomous autorotation
	};

	const string AP_COPTER_CUSTOM_MODE_NAME[28] =
		{"STABILIZE", "ACRO", "ALT_HOLD", "AUTO", "GUIDED", "LOITER", "RTL", "CIRCLE",
		 "?", "LAND", "?", "DRIFT", "?", "SPORT", "FLIP", "AUTOTUNE", "POSHOLD",
		 "BRAKE", "THROW", "AVOID_ADSB", "GUIDED_NOGPS", "SMART_RTL", "FLOWHOLD",
		 "FOLLOW", "ZIGZAG", "SYSTEMID", "AUTOROTATE"};

	class _APmav_copter : public _APmav_base
	{
	public:
		_APmav_copter();
		~_APmav_copter();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(void) override;
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);
		virtual void console(const json &j, void *pJSONbase);

		// copter specific
		virtual string getModeName(void);
		virtual void takeOff(float alt);

	protected:
		virtual void updateApMavSend(void);
		virtual void updateApMavRecv(void);

	private:
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_APmav_copter *)This)->update();
			return NULL;
		}

	protected:

	};

}
#endif

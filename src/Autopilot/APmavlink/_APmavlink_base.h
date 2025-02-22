#ifndef OpenKAI_src_Autopilot_APmavlink__APmavlink_base_H_
#define OpenKAI_src_Autopilot_APmavlink__APmavlink_base_H_

#include "../../Protocol/_Mavlink.h"
#include "../../State/_StateControl.h"
#include "../../Utility/utilEvent.h"
#include "../../Utility/utilVar.h"

#define AP_N_CUSTOM_MODE 28

namespace kai
{

	enum AP_TYPE
	{
		ardupilot_copter = 0,
		ardupilot_rover = 1
	};

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
		 "?", "AUTO", "RTL", "SMART_RTL", "?", "?", "GUIDED", "INITIALISING",
		 "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?"};

	struct AP_MOUNT
	{
		bool m_bEnable;
		mavlink_mount_control_t m_control;
		mavlink_mount_configure_t m_config;

		void init(void)
		{
			m_bEnable = false;
			m_control.input_a = 0; //pitch
			m_control.input_b = 0; //roll
			m_control.input_c = 0; //yaw
			m_control.save_position = 0;
			m_config.stab_pitch = 0;
			m_config.stab_roll = 0;
			m_config.stab_yaw = 0;
			m_config.mount_mode = 2;
		}
	};

	class _APmavlink_base : public _ModuleBase
	{
	public:
		_APmavlink_base();
		~_APmavlink_base();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);
		virtual void update(void);
		virtual void console(void *pConsole);

		// mode
		void setMode(uint32_t iMode);
		int getMode(void);
		virtual string getModeName(void);

		void setArm(bool bArm);
		bool bArmed(void);
		void takeOff(float alt);

		// gimbal, payloads
		virtual void setMount(AP_MOUNT &mount);

		// status
		int getGPSfixType(void);
		int getGPShacc(void);
		vDouble3 getHomePos(void);
		vDouble4 getGlobalPos(void);
		float getHdg(void);
		vFloat3 getSpeed(void);
		vFloat3 getAttitude(void);
		float getBattery(void);

		// mission
		int getWPseq(void);
		int getWPtotal(void);

		// Mavlink
		_Mavlink* getMavlink(void);

	protected:
		void updateModeSync(void);
		void updateBase(void);
		static void *getUpdate(void *This)
		{
			((_APmavlink_base *)This)->update();
			return NULL;
		}

	protected:
		_Mavlink *m_pMav;
		AP_TYPE m_apType;
		// int m_apMode;
		// bool m_bApArmed;

		bool m_bHomeSet;
		vDouble3 m_vHomePos;
		vDouble4 m_vGlobalPos; //lat, lon, alt, relative_alt in meters
		vDouble3 m_vLocalPos;
		vFloat3 m_vSpeed;
		vFloat3 m_vAtti; //yaw, pitch, roll
		float m_apHdg;	 //heading in degree
		float m_battery; //remaining percentage
		int8_t m_gpsFixType;
		int m_gpsHacc;

		INTERVAL_EVENT m_ieSendHB;
		INTERVAL_EVENT m_ieSendMsgInt;

		// set vars
		bool m_bSyncMode;
		VAR_WR<uint32_t> m_wrApMode;
		VAR_WR<bool> m_wrbArm;

	};

}
#endif

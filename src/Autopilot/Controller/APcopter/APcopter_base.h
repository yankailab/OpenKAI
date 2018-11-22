#ifndef OpenKAI_src_Autopilot_Controller_APcopter_base_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_base_H_

#include "../../../Base/common.h"
#include "../../../Protocol/_Mavlink.h"
#include "../../../Mission/_MissionControl.h"
#include "../../ActionBase.h"

#define AP_N_CUSTOM_MODE 24

namespace kai
{
// Auto Pilot Modes enumeration
enum custom_mode_t {
    STABILIZE =     0,  // manual airframe angle with manual throttle
    ACRO =          1,  // manual body-frame angular rate with manual throttle
    ALT_HOLD =      2,  // manual airframe angle with automatic throttle
    AUTO =          3,  // fully automatic waypoint control using mission commands
    GUIDED =        4,  // fully automatic fly to coordinate or fly at velocity/direction using GCS immediate commands
    LOITER =        5,  // automatic horizontal acceleration with automatic throttle
    RTL =           6,  // automatic return to launching point
    CIRCLE =        7,  // automatic circular flight with automatic throttle
    LAND =          9,  // automatic landing with horizontal position control
    DRIFT =        11,  // semi-automous position, yaw and throttle control
    SPORT =        13,  // manual earth-frame angular rate control with manual throttle
    FLIP =         14,  // automatically flip the vehicle on the roll axis
    AUTOTUNE =     15,  // automatically tune the vehicle's roll and pitch gains
    POSHOLD =      16,  // automatic position hold with manual override, with automatic throttle
    BRAKE =        17,  // full-brake using inertial/GPS system, no pilot input
    THROW =        18,  // throw to launch mode using inertial/GPS system, no pilot input
    AVOID_ADSB =   19,  // automatic avoidance of obstacles in the macro scale - e.g. full-sized aircraft
    GUIDED_NOGPS = 20,  // guided mode but only accepts attitude and altitude
    SMART_RTL =    21,  // SMART_RTL returns to home by retracing its steps
    FLOWHOLD  =    22,  // FLOWHOLD holds position with optical flow without rangefinder
    FOLLOW    =    23,  // follow attempts to follow another vehicle or ground station
};

const string custom_mode_name[] =
{
"STABILIZE",
"ACRO",
"ALT_HOLD",
"AUTO",
"GUIDED",
"LOITER",
"RTL",
"CIRCLE",
"LAND",
"DRIFT",
"SPORT",
"FLIP",
"AUTOTUNE",
"POSHOLD",
"BRAKE",
"THROW",
"AVOID_ADSB",
"GUIDED_NOGPS",
"SMART_RTL",
"FLOWHOLD",
"FOLLOW",
};


class APcopter_base: public ActionBase
{
public:
	APcopter_base();
	~APcopter_base();

	bool init(void* pKiss);
	int check(void);
	void update(void);
	bool draw(void);
	bool cli(int& iY);

	void setApMode(uint32_t iMode);
	uint32_t getApMode(void);
	void setApArm(bool bArm);
	bool getApArm(void);
	bool bApModeChanged(void);

	uint32_t apMode(void);
	string apModeName(void);

public:
	_Mavlink* m_pMavlink;
	uint64_t m_lastHeartbeat;
	uint64_t m_iHeartbeat;

	uint32_t m_apMode;
	uint32_t m_lastApMode;
	bool	 m_bApModeChanged;

	int m_freqRawSensors;
	int m_freqExtStat;
	int m_freqRC;
	int m_freqPos;
	int m_freqExtra1;

	int m_freqSendHeartbeat;
};

}
#endif

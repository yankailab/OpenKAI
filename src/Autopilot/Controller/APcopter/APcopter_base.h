#ifndef OpenKAI_src_Autopilot_Controller_APcopter_base_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_base_H_

#include "../../../Base/common.h"
#include "../../../Protocol/_Mavlink.h"
#include "../../../Control/PIDctrl.h"
#include "../../ActionBase.h"

namespace kai
{

class APcopter_base: public ActionBase
{
public:
	APcopter_base();
	~APcopter_base();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

public:
	PIDctrl* m_pRoll;
	PIDctrl* m_pPitch;
	PIDctrl* m_pYaw;
	PIDctrl* m_pAlt;

	_Mavlink* m_pMavlink;
	uint64_t m_lastHeartbeat;
	uint64_t m_iHeartbeat;
	uint32_t m_flightMode;

	int m_freqAtti;
	int m_freqGlobalPos;
	int m_freqHeartbeat;

};

/*
// Auto Pilot Modes enumeration
enum control_mode_t {
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
};
 */
}
#endif

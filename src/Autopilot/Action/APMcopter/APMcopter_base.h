#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_APMCOPTER_BASE_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_APMCOPTER_BASE_H_

#include "../../../Base/common.h"
#include "../../../Protocol/_Mavlink.h"
#include "../ActionBase.h"

namespace kai
{

struct APMcopter_PID
{
	double m_P;
	double m_I;
	double m_Imax;
	double m_D;
	double m_dT;
	double m_rate;
	double m_lim;

	void reset(void)
	{
		m_P = 0;
		m_I = 0;
		m_Imax = 0;
		m_D = 0;
		m_dT = 0;
		m_rate = 0;
		m_lim = 100.0;
	}
};

struct APMcopter_CTRL
{
	double m_v;
	double m_pos;
	double m_predPos;
	double m_targetPos;

	double m_err;
	double m_errOld;
	double m_errInteg;

	void resetErr(void)
	{
		m_err = 0;
		m_errOld = 0;
		m_errInteg = 0;
	}

	void reset(void)
	{
		resetErr();
		m_pos = 0;
		m_predPos = 0;
		m_targetPos = 0;
	}
};

struct DISTANCE_SENSOR
{
	uint8_t m_type;
	uint8_t m_orientation;
	uint16_t m_maxDistance;	//unit: centimeters
	uint16_t m_minDistance;
	uint16_t m_distance;
};

class APMcopter_base: public ActionBase
{
public:
	APMcopter_base();
	~APMcopter_base();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

	void sendHeartbeat(void);
	void updateDistanceSensor(DISTANCE_SENSOR* pSensor);

public:
	_Mavlink* m_pMavlink;
	uint64_t m_lastHeartbeat;
	uint64_t m_iHeartbeat;
	uint32_t m_flightMode;

	APMcopter_PID m_pidRoll;
	APMcopter_PID m_pidPitch;
	APMcopter_PID m_pidYaw;
	APMcopter_PID m_pidThr;

	APMcopter_CTRL m_ctrlRoll;
	APMcopter_CTRL m_ctrlPitch;
	APMcopter_CTRL m_ctrlYaw;
	APMcopter_CTRL m_ctrlThr;

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


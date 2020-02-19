#ifndef OpenKAI_src_Protocol__CETCUS_H_
#define OpenKAI_src_Protocol__CETCUS_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../IO/_TCPclient.h"

using namespace picojson;

namespace kai
{

enum CETCUS_STATE
{
	cetcus_connecting = 0,
	cetcus_planeConnected = 1,
	cetcus_receivedMission = 2,
	cetcus_startFly = 3,
};

enum CETCUS_FLY_STATUS
{
	cetcus_standby = 0,
	cetcus_takeOff = 1,
	cetcus_climb = 2,
	cetcus_cruise = 3,
	cetcus_land = 4,
	cetcus_wait = 5,
	cetcus_parachute = 6,
	cetcus_diversion = 7,
	cetcus_return = 8,
};

enum CETCUS_STATUS
{
	cetcus_normal = 0,
	cetcus_failure = 1,
};

enum CETCUS_CMD
{
	cetcus_cmd_unlock = 1,
	cetcus_cmd_takeOff = 2,
	cetcus_cmd_diversion = 3,
	cetcus_cmd_return = 4,
	cetcus_cmd_land = 5,
	cetcus_cmd_wait = 6,
};

struct CETCUS_WAYPOINT
{
	string m_name;
	int m_id;
	vDouble4 m_vPos; //lat, lon, alt, ground_alt

	void init(void)
	{
		m_name = "";
		m_id = 0;
		m_vPos.init();
	}
};

class _CETCUS: public _ThreadBase
{
public:
	_CETCUS();
	~_CETCUS();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void draw(void);

private:
	//send
	void updateMission(void);
	void planeConnect(void);
	void startFly(void);
	void detailUpload(void);
	void endFly(void);

	//receive
	bool recv();
	void handleMsg(string& str);
	void planeConnect(picojson::object& jo);
	void flightSend(picojson::object& jo);
	void startFly(picojson::object& jo);
	void detailUpload(picojson::object& jo);
	void command(picojson::object& jo);
	void endFly(picojson::object& jo);


	void updateW(void);
	static void* getUpdateThreadW(void* This)
	{
		((_CETCUS*) This)->updateW();
		return NULL;
	}

	void updateR(void);
	static void* getUpdateThreadR(void* This)
	{
		((_CETCUS*) This)->updateR();
		return NULL;
	}

public:
	_IOBase* m_pIO;

	string m_msgFinishSend;
	string m_msgFinishRecv;

	CETCUS_STATE m_state;
	string m_uavNo;
	string m_airlineNo;
	string m_airlineName;
	string m_startTime;
	string m_endTime;
	string m_flyID;
	CETCUS_FLY_STATUS m_flyStatus;
	vDouble4 m_vPos;	//lat, lon, alt, ground_alt
	vFloat3 m_vAtti;	//yaw, pitch, roll
	float	m_course;
	vDouble4 m_vSpeed;	//velocity_c, true_airspeed, ground_speed
	float m_remainOil;
	float m_remainDist;
	float m_remainTime;
	CETCUS_STATUS m_statusMot;
	CETCUS_STATUS m_statusNav;
	CETCUS_STATUS m_statusCom;

	CETCUS_WAYPOINT m_wpStart;
	CETCUS_WAYPOINT m_wpEnd;
	vector<CETCUS_WAYPOINT> m_vWP;


	pthread_t m_rThreadID;
	bool	m_bRThreadON;


};

}

#endif

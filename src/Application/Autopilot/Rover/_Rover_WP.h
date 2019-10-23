#ifndef OpenKAI_src_Autopilot_Rover__Rover_WP_H_
#define OpenKAI_src_Autopilot_Rover__Rover_WP_H_

#include "../../../Detector/_DetectorBase.h"
#include "_Rover_base.h"

namespace kai
{

class _Rover_WP: public _AutopilotBase
{
public:
	_Rover_WP();
	~_Rover_WP();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void update(void);
	void draw(void);

private:
	void updateWP(void);
	void updateRecord(void);
	static void* getUpdateThread(void* This)
	{
		((_Rover_WP*) This)->update();
		return NULL;
	}

public:
	_Mavlink* m_pMavlink;
	ROVER_CTRL m_ctrl;

	float m_nSpeed;
	vDouble4 m_vPos;

	vector<MISSION_WAYPOINT> m_vWP;
	double m_dRecP;
	double m_err;

};

}
#endif

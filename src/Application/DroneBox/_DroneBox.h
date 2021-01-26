#ifndef OpenKAI_src_Application_DroneBox__DroneBox_H_
#define OpenKAI_src_Application_DroneBox__DroneBox_H_

#include "../../Mission/_MissionBase.h"
#include "../../Protocol/_Modbus.h"

namespace kai
{

struct DRONEBOX_STATE 
{
    int8_t STANDBY;
	int8_t LANDING_REQUEST;
    int8_t LANDING_READY;
	int8_t LANDING_COMPLETE;
    int8_t TAKEOFF_REQUEST;
    int8_t TAKEOFF_READY;
	int8_t TAKEOFF_COMPLETE;

	bool bValid(void)
	{
		IF_F(STANDBY < 0);
		IF_F(LANDING_REQUEST < 0);
		IF_F(LANDING_READY < 0);
		IF_F(LANDING_COMPLETE < 0);
		IF_F(TAKEOFF_REQUEST < 0);
		IF_F(TAKEOFF_READY < 0);
		IF_F(TAKEOFF_COMPLETE < 0);

		return true;
	}
	
	bool assign(_MissionControl* pMC)
    {
        NULL_F(pMC);
        STANDBY = pMC->getMissionIdx("STANDBY");
        LANDING_REQUEST = pMC->getMissionIdx("LANDING_REQUEST");
        LANDING_READY = pMC->getMissionIdx("LANDING_READY");
        LANDING_COMPLETE = pMC->getMissionIdx("LANDING_COMPLETE");
        TAKEOFF_REQUEST = pMC->getMissionIdx("TAKEOFF_REQUEST");
        TAKEOFF_READY = pMC->getMissionIdx("TAKEOFF_READY");
        TAKEOFF_COMPLETE = pMC->getMissionIdx("TAKEOFF_COMPLETE");
        
        return bValid();
    }
};

class _DroneBox: public _MissionBase
{
public:
	_DroneBox();
	~_DroneBox();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void update(void);
	void draw(void);
    
    bool landingRequest(int vID);
    bool bLandingReady(int vID);
    void landingStatus(int vID);
    bool takeoffRequest(int vID);
    bool bTakeoffReady(int vID);
    void takeoffStatus (int vID);

    //Drone Box mechanical control
    void boxLandingPrepare (void);
    bool bBoxLandingReady (void);
    void boxLandingComplete (void);
    void boxTakeoffPrepare (void);
    bool bBoxTakeoffReady (void);
    void boxTakeoffComplete (void);
    void boxRecover (void);

protected:
    void updateBox (void);
	static void* getUpdateThread(void* This)
	{
		((_DroneBox*) This)->update();
		return NULL;
	}

public:
    _Modbus* m_pMB;
	int		m_iSlave;

    int m_iBox;      //base ID
    int m_iVehicle;  //vehicle ID
    int m_iState;
    DRONEBOX_STATE m_state;
};

}
#endif

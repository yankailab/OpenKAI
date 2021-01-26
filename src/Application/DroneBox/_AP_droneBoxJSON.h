#ifndef OpenKAI_src_Application_DroneBox__AP_droneBoxJSON_H_
#define OpenKAI_src_Application_DroneBox__AP_droneBoxJSON_H_

#include "../../Protocol/_JSONbase.h"
#include "../../Application/Autopilot/ArduPilot/_AP_base.h"

using namespace picojson;

namespace kai
{

class _AP_droneBoxJSON : public _JSONbase
{
public:
	_AP_droneBoxJSON();
	~_AP_droneBoxJSON();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void draw(void);

    //msg
    bool sendMsg (picojson::object& o);
    void sendHeartbeat (void);
    
protected:
    void heartbeat(picojson::object& o);
    void landingRequest (picojson::object& o);
    void landingStatus (picojson::object& o);
    void takeoffRequest (picojson::object& o);
    void takeoffStatus (picojson::object& o);
    
	virtual void send(void);
	virtual void handleMsg(string& str);

private:
	void updateW(void);
	static void* getUpdateThreadW(void* This)
	{
		((_AP_droneBoxJSON*) This)->updateW();
		return NULL;
	}

	void updateR(void);
	static void* getUpdateThreadR(void* This)
	{
		((_AP_droneBoxJSON*) This)->updateR();
		return NULL;
	}

public:
    _AP_base* m_pAP;

    INTERVAL_EVENT m_tIntHeartbeat;
    
};

}
#endif

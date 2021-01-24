#ifndef OpenKAI_src_Application_DroneBox__DroneBoxJSON_H_
#define OpenKAI_src_Application_DroneBox__DroneBoxJSON_H_

#include "../../Protocol/_JSONbase.h"

using namespace picojson;

namespace kai
{

class _DroneBoxJSON : public _JSONbase
{
public:
	_DroneBoxJSON();
	~_DroneBoxJSON();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void draw(void);

    //msg
    void sendHeartbeat (void);
    bool sendMsg (picojson::object& o);
    
protected:
    void heartbeat(picojson::object& o);
    void landingRequest (picojson::object& o);
    void landingStatus (picojson::object& o);
    void takeoffRequest (picojson::object& o);
    void takeoffStatus (picojson::object& o);
    void ackLandingRequest(picojson::object& o);
    void ackTakeoffRequest(picojson::object& o);
    
	virtual void send(void);
	virtual void handleMsg(string& str);

private:
	void updateW(void);
	static void* getUpdateThreadW(void* This)
	{
		((_DroneBoxJSON*) This)->updateW();
		return NULL;
	}

	void updateR(void);
	static void* getUpdateThreadR(void* This)
	{
		((_DroneBoxJSON*) This)->updateR();
		return NULL;
	}

public:
    INTERVAL_EVENT m_tIntHeartbeat;
    
};

}
#endif

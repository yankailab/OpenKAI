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

    //general
    void heartbeat(void);
    
    //vehicle to base
    void actionReuqest(void);  //hatch open, platform up, if approved
    void actionComplete(void); //platform down, hatch close

    //base to vehicle
    void actionReply(void);

protected:
    void heartbeat(picojson::object& jo);
    void actionReuqest(picojson::object& jo);
    void actionComplete(picojson::object& jo);
    void actionReply(picojson::object& jo);
    
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

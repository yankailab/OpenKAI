#ifndef OpenKAI_src_Application_DroneBox__DroneBoxJSON_H_
#define OpenKAI_src_Application_DroneBox__DroneBoxJSON_H_

#include "../../Protocol/_JSONbase.h"
#include "_DroneBox.h"

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

protected:
	void send(void);

    //msg handlers
	void handleMsg(string& str);
    void heartbeat(picojson::object& o);
    void landingRequest (picojson::object& o);
    void landingStatus (picojson::object& o);
    void takeoffRequest (picojson::object& o);
    void takeoffStatus (picojson::object& o);
    
private:
	void updateW(void);
	static void* getUpdateW(void* This)
	{
		((_DroneBoxJSON*) This)->updateW();
		return NULL;
	}

	void updateR(void);
	static void* getUpdateR(void* This)
	{
		((_DroneBoxJSON*) This)->updateR();
		return NULL;
	}

public:
    _DroneBox* m_pDB;
};

}
#endif

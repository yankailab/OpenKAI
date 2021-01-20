#ifndef OpenKAI_src_Application_DroneBox__DroneBox_H_
#define OpenKAI_src_Application_DroneBox__DroneBox_H_

#include "../../Mission/_MissionBase.h"
#include "../../Protocol/_Modbus.h"
#include "_DroneBoxJSON.h"

namespace kai
{

struct DRONEBOX_STATE 
{
	int8_t STORING;
	int8_t STORE;
	int8_t OPENING;
	int8_t OPEN;

	bool bValid(void)
	{
		IF_F(STORING < 0);
		IF_F(STORE < 0);
		IF_F(OPENING < 0);
		IF_F(OPEN < 0);

		return true;
	}
	
	bool assign(_MissionControl* pMC)
    {
        NULL_F(pMC);
        STORING = pMC->getMissionIdx("STORING");
        STORE = pMC->getMissionIdx("STORE");
        OPENING = pMC->getMissionIdx("OPENING");
        OPEN = pMC->getMissionIdx("OPEN");
        
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

private:
	void updateBox (void);
	static void* getUpdateThread(void* This)
	{
		((_DroneBox*) This)->update();
		return NULL;
	}

public:
	_DroneBoxJSON* m_pJvehicle;
    _Modbus* m_pMB;
    
    int8_t m_iHatch;
    int8_t m_iPlatform;
    DRONEBOX_STATE m_sHatch;
    DRONEBOX_STATE m_sPlatform;
     
};

}
#endif

#ifndef OpenKAI_src_Application_DroneBox__DroneBox_H_
#define OpenKAI_src_Application_DroneBox__DroneBox_H_

#include "../../GCS/_GCSbase.h"
#include "../../Protocol/_Modbus.h"

namespace kai
{

class _DroneBox: public _GCSbase
{
public:
	_DroneBox();
	~_DroneBox();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void update(void);
	void draw(void);
    
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
};

}
#endif

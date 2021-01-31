#ifndef OpenKAI_src_Application_Autopilot_ArduPilot__AP_gcs_H_
#define OpenKAI_src_Application_Autopilot_ArduPilot__AP_gcs_H_

#include "_AP_base.h"
#include "../../../GCS/_GCSbase.h"

namespace kai
{

class _AP_gcs: public _GCSbase
{
public:
	_AP_gcs();
	~_AP_gcs();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void update(void);
	virtual void draw(void);
    
    void landingReady(bool bReady);
    void takeoffReady(bool bReady);
    
protected:
    virtual void updateGCS (void);
	static void* getUpdateThread(void* This)
	{
		(( _AP_gcs*) This)->update();
		return NULL;
	}

public:
    _AP_base* m_pAP;
    
    float m_altAirborne;
};

}
#endif

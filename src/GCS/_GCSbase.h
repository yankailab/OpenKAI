#ifndef OpenKAI_src_GCS__GCSbase_H_
#define OpenKAI_src_GCS__GCSbase_H_

#include "../State/_StateBase.h"

namespace kai
{

struct GCS_STATE
{
    int8_t m_iState;
    int8_t STANDBY;
    int8_t TAKEOFF_REQUEST;
    int8_t TAKEOFF_READY;
    int8_t AIRBORNE;
	int8_t LANDING_REQUEST;
    int8_t LANDING_READY;
    int8_t LANDING;
    int8_t LANDED;
    
	bool assign(_StateControl* pSC )
    {
        NULL_F( pSC );

        m_iState = -1;
        STANDBY = pSC->getStateIdxByName ("STANDBY");
        TAKEOFF_REQUEST = pSC->getStateIdxByName ("TAKEOFF_REQUEST");
        TAKEOFF_READY = pSC->getStateIdxByName ("TAKEOFF_READY");
        AIRBORNE = pSC->getStateIdxByName ("AIRBORNE");
        LANDING_REQUEST = pSC->getStateIdxByName ("LANDING_REQUEST");
        LANDING_READY = pSC->getStateIdxByName ("LANDING_READY");
        LANDING = pSC->getStateIdxByName ("LANDING");
        LANDED = pSC->getStateIdxByName ("LANDED");
        
        return bValid();
    }

    bool bValid(void)
	{
		IF_F(STANDBY < 0);
		IF_F(TAKEOFF_REQUEST < 0);
		IF_F(TAKEOFF_READY < 0);
		IF_F(AIRBORNE < 0);
		IF_F(LANDING_REQUEST < 0);
		IF_F(LANDING_READY < 0);
		IF_F(LANDING < 0);
		IF_F(LANDED < 0);

		return true;
	}
	
	void update(_StateControl* pSC)
    {
        NULL_(pSC);
        m_iState = pSC->getStateIdx();
    }
	
	bool bSTANDBY(void)
    {
        return (m_iState == STANDBY);
    }
    
   	bool bTAKEOFF_REQUEST(void)
    {
        return (m_iState == TAKEOFF_REQUEST);
    }

	bool bTAKEOFF_READY(void)
    {
        return (m_iState == TAKEOFF_READY);
    }

   	bool bAIRBORNE(void)
    {
        return (m_iState == AIRBORNE);
    }

   	bool bLANDING_REQUEST(void)
    {
        return (m_iState == LANDING_REQUEST);
    }

   	bool bLANDING_READY(void)
    {
        return (m_iState == LANDING_READY);
    }
    
    bool bLANDING(void)
    {
        return (m_iState == LANDING);
    }

    bool bLANDED(void)
    {
        return (m_iState == LANDED);
    }

};

class _GCSbase: public _StateBase
{
public:
	_GCSbase();
	~_GCSbase();

	virtual bool init(void* pKiss);
	virtual int check(void);
	virtual void draw(void);
    
    virtual int getID (void);
    virtual GCS_STATE* getState(void);

    virtual void status (int vID, const string& stat);
    virtual bool takeoffRequest(int vID);
    virtual bool landingRequest(int vID);

protected:
    virtual void updateGCS (void);

public:
    int m_gcsID;

    GCS_STATE m_state;
};

}
#endif

#ifndef OpenKAI_src_GCS__DroneBoxState_H_
#define OpenKAI_src_GCS__DroneBoxState_H_

#include "../../State/_StateControl.h"

namespace kai
{

    struct DRONEBOX_STATE
    {
        int8_t m_iState;
        int8_t STANDBY;
        int8_t TAKEOFF_REQUEST;
        int8_t TAKEOFF_READY;
        int8_t AIRBORNE;
        int8_t LANDING_REQUEST;
        int8_t LANDING;
        int8_t TOUCHDOWN;
        int8_t LANDED;

        bool assign(_StateControl *pSC)
        {
            NULL_F(pSC);

            m_iState = -1;
            STANDBY = pSC->getStateIdxByName("STANDBY");
            TAKEOFF_REQUEST = pSC->getStateIdxByName("TAKEOFF_REQUEST");
            TAKEOFF_READY = pSC->getStateIdxByName("TAKEOFF_READY");
            AIRBORNE = pSC->getStateIdxByName("AIRBORNE");
            LANDING_REQUEST = pSC->getStateIdxByName("LANDING_REQUEST");
            LANDING = pSC->getStateIdxByName("LANDING");
            TOUCHDOWN = pSC->getStateIdxByName("TOUCHDOWN");
            LANDED = pSC->getStateIdxByName("LANDED");

            return bValid();
        }

        bool bValid(void)
        {
            IF_F(STANDBY < 0);
            IF_F(TAKEOFF_REQUEST < 0);
            IF_F(TAKEOFF_READY < 0);
            IF_F(AIRBORNE < 0);
            IF_F(LANDING_REQUEST < 0);
            IF_F(LANDING < 0);
            IF_F(TOUCHDOWN < 0);
            IF_F(LANDED < 0);

            return true;
        }

        void update(_StateControl *pSC)
        {
            NULL_(pSC);
            m_iState = pSC->getCurrentStateIdx();
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

        bool bLANDING(void)
        {
            return (m_iState == LANDING);
        }

        bool bTOUCHDOWN(void)
        {
            return (m_iState == TOUCHDOWN);
        }

        bool bLANDED(void)
        {
            return (m_iState == LANDED);
        }
    };

    class _DroneBoxState : public _ModuleBase
    {
    public:
        _DroneBoxState();
        ~_DroneBoxState();

        virtual bool init(void *pKiss);
		virtual bool link(void);
        virtual int check(void);

        virtual int getID(void);
        virtual DRONEBOX_STATE *getState(void);

        virtual void setState(int vID, const string &stat);
        virtual bool takeoffRequest(int vID);
        virtual bool landingRequest(int vID);

    protected:
        virtual void updateDroneBox(void);

    public:
		_StateControl* m_pSC;

        int m_gcsID;
        DRONEBOX_STATE m_state;
    };

}
#endif

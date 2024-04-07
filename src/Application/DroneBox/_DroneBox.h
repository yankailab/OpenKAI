#ifndef OpenKAI_src_Application_DroneBox__DroneBox_H_
#define OpenKAI_src_Application_DroneBox__DroneBox_H_

#include "../../Protocol/_Modbus.h"
#include "_DroneBoxState.h"

namespace kai
{

    enum DRONEBOX_LAST_CMD
    {
        dbx_unknown = -1,
        dbx_landingPrepare = 0,
        dbx_bLandingReady = 1,
        dbx_landingComplete = 2,
        dbx_takeoffPrepare = 3,
        dbx_bTakeoffReady = 4,
        dbx_takeoffComplete = 5,
        dbx_boxRecover = 6,
    };

    class _DroneBox : public _DroneBoxState
    {
    public:
        _DroneBox();
        ~_DroneBox();

        virtual bool init(void *pKiss);
		virtual bool link(void);
        virtual bool start(void);
        virtual int check(void);
        virtual void update(void);

        // general
        int getID(void);
        vDouble2 getPos(void);

        //Drone Box mechanical control
        bool boxLandingPrepare(void);
        bool bBoxLandingReady(void);
        bool boxLandingComplete(void);
        bool boxTakeoffPrepare(void);
        bool bBoxTakeoffReady(void);
        bool boxTakeoffComplete(void);
        bool boxRecover(void);

    protected:
        virtual void updateDroneBox(void);
        static void *getUpdate(void *This)
        {
            ((_DroneBox *)This)->update();
            return NULL;
        }

    public:
        _Modbus *m_pMB;
        int m_iSlave;

        DRONEBOX_LAST_CMD m_lastCMD;

        int m_ID;
        vDouble2 m_vPos;
    };

}
#endif

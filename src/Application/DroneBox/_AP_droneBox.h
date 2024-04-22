#ifndef OpenKAI_src_Application_DroneBox__AP_droneBox_H_
#define OpenKAI_src_Application_DroneBox__AP_droneBox_H_

//#include "../../Autopilot/ArduPilot/_AP_land.h"
#include "../../Autopilot/ArduPilot/_AP_landingTarget.h"
#include "../../State/_StateControl.h"
#include "_DroneBoxState.h"

namespace kai
{

	class _AP_droneBox : public _DroneBoxState
	{
	public:
		_AP_droneBox();
		~_AP_droneBox();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);
		virtual void update(void);

		void addTargetDroneBox(int id, vDouble2 vPdb);
		int getTargetDroneBoxID(void);

		void landingReady(bool bReady);
		void takeoffReady(bool bReady);

	protected:
		virtual void updateDroneBox(void);
		static void *getUpdate(void *This)
		{
			((_AP_droneBox *)This)->update();
			return NULL;
		}

	public:
		_AP_base *m_pAP;
		// _AP_land *m_pAPland;

		bool m_bAutoArm;
		float m_altTakeoff;
		float m_altLand;
		int m_dLanded;

		int m_targetDroneBoxID;
		vDouble2 m_vTargetDroneBoxPos;
		string m_boxState;
	};

}
#endif

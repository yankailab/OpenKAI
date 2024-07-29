#ifndef OpenKAI_src_Application_DroneBox__AP_droneBox_H_
#define OpenKAI_src_Application_DroneBox__AP_droneBox_H_

//#include "../../Autopilot/ArduPilot/_AP_land.h"
#include "../../Autopilot/ArduPilot/_AP_landingTarget.h"
#include "../../State/_StateControl.h"

namespace kai
{

	class _AP_droneBox : public _ModuleBase
	{
	public:
		_AP_droneBox();
		~_AP_droneBox();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);
		virtual void update(void);

		string getState(void);
		void landingReady(bool bReady);
		void takeoffReady(bool bReady);
		void setBoxState(const string& s);

	protected:
		virtual void updateAPdroneBox(void);
		static void *getUpdate(void *This)
		{
			((_AP_droneBox *)This)->update();
			return NULL;
		}

	protected:
		_StateControl* m_pSC;
		_AP_base *m_pAP;

		int m_gpsToutSec;
		int m_gpsHaccMax;
		uint64_t m_tGPSmeasureStart;
		
		bool m_bAutoArm;
		float m_altTakeoff;
		float m_altLand;

		string m_boxState;
	};

}
#endif

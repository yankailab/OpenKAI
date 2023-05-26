#ifndef OpenKAI_src_Application_DroneFleet__AP_fleet_H_
#define OpenKAI_src_Application_DroneFleet__AP_fleet_H_

#include "../../Autopilot/ArduPilot/_AP_base.h"

namespace kai
{

	class _AP_fleet : public _StateBase
	{
	public:
		_AP_fleet();
		~_AP_fleet();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);
		virtual void update(void);
		virtual void console(void *pConsole);

	protected:

	private:
		void updateFleet(void);
		static void *getUpdate(void *This)
		{
			((_AP_fleet *)This)->update();
			return NULL;
		}

	public:
		_AP_base *m_pAP;
		
	};

}
#endif

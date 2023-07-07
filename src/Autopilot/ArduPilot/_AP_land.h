#ifndef OpenKAI_src_Autopilot_AP__AP_land_H_
#define OpenKAI_src_Autopilot_AP__AP_land_H_

#include "../../Detector/_DetectorBase.h"
#include "../../Sensor/Distance/_DistSensorBase.h"
#include "_AP_follow.h"

namespace kai
{

	class _AP_land : public _AP_follow
	{
	public:
		_AP_land();
		~_AP_land();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);
		virtual void update(void);
		virtual void console(void *pConsole);
		virtual void draw(void *pFrame);

		bool bComplete(void);

	protected:
		virtual bool updateTarget(void);
		virtual bool findTarget(void);
		static void *getUpdate(void *This)
		{
			((_AP_land *)This)->update();
			return NULL;
		}

	public:
		_DistSensorBase* m_pDS;

		float m_zrK;
		float m_rAlt;
		float m_rAltComplete;
		float m_dTarget; //dist to target
		float m_dTargetComplete;
		float m_rTargetComplete;
		bool m_bRtargetComplete;
	};
}
#endif

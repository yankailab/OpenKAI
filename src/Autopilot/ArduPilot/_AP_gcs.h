#ifndef OpenKAI_src_Application_Autopilot_ArduPilot__AP_gcs_H_
#define OpenKAI_src_Application_Autopilot_ArduPilot__AP_gcs_H_

#include "_AP_base.h"
#include "_AP_land.h"
#include "../../GCS/_GCSbase.h"

namespace kai
{

	class _AP_gcs : public _GCSbase
	{
	public:
		_AP_gcs();
		~_AP_gcs();

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
		virtual void updateGCS(void);
		static void *getUpdate(void *This)
		{
			((_AP_gcs *)This)->update();
			return NULL;
		}

	public:
		_AP_base *m_pAP;
		_AP_land *m_pAPland;

		bool m_bAutoArm;
		float m_altTakeoff;
		float m_altLand;
		int m_dLanded;

		int m_targetDroneBoxID;
		vDouble2 m_vTargetDroneBoxPos;
	};

}
#endif

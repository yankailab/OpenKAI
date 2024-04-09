#ifndef OpenKAI_src_Autopilot_AP__AP_land_H_
#define OpenKAI_src_Autopilot_AP__AP_land_H_

#include "_AP_follow.h"
#include "../../Detector/_DetectorBase.h"
#include "../../Utility/utilEvent.h"
#include "../../Sensor/Distance/_DistSensorBase.h"

namespace kai
{
	struct AP_LAND_TAG
	{
		int m_id = -1;
		int m_priority = 0;
		vFloat2 m_vSize = {0, FLT_MAX}; // effective size range
		vFloat2 m_vKdist; // translate size into distance

		float getDist(float s)
		{
			float c = m_vSize.constrain(s);
			return ((m_vSize.y - c) / m_vSize.len()) * m_vKdist.len() + m_vKdist.x;
		}
	};

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

		bool bComplete(void);

	protected:
		virtual void onSleep(void);

		virtual void updatePID(void);
		virtual AP_LAND_TAG* getTag(int id);
		virtual bool findTag(void);
		virtual void updateMove(void);
		static void *getUpdate(void *This)
		{
			((_AP_land *)This)->update();
			return NULL;
		}

	protected:
		_DistSensorBase* m_pDS;
		vFloat2 m_vDSrange;
		
		vector<AP_LAND_TAG> m_vTags;
		AP_LAND_TAG* m_pTag;
		vFloat2 m_vFov; // cam FOV horiz/vert

		vFloat4 m_vComplete; // complete condition for vPtarget
		float m_zrK;
	};
}
#endif

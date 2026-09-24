#ifndef OpenKAI_src_Autopilot_ArduPilot__APmav_land_H_
#define OpenKAI_src_Autopilot_ArduPilot__APmav_land_H_

#include "_APmav_follow.h"
#include "../../Detector/_DetectorBase.h"
#include "../../Utility/utilEvent.h"
#include "../../Sensor/Distance/_DistSensorBase.h"

namespace kai
{
	struct AP_LAND_TAG
	{
		int m_id = -1;
		int m_priority = 0;
		Vector2f m_vSize = {0, FLT_MAX}; // effective size range
		Vector2f m_vKdist = Vector2f::Zero(); // translate size into distance

		float getDist(float s)
		{
			float c = std::clamp(s, m_vSize.x(), m_vSize.y());
			return ((m_vSize.y() - c) / m_vSize.norm()) * m_vKdist.norm() + m_vKdist.x();
		}
	};

	class _APmav_land : public _APmav_follow
	{
	public:
		_APmav_land();
		~_APmav_land();

		virtual bool init(const json& j);
		virtual bool link(const json& j, ModuleMgr* pM);
		virtual bool start(void);
		virtual bool check(void);
		virtual void update(void);

		bool bComplete(void);

	protected:
		virtual void onPause(void);

		virtual void updatePID(void);
		virtual AP_LAND_TAG* getTag(int id);
		virtual bool findTag(void);
		virtual void updateMove(void);
		static void *getUpdate(void *This)
		{
			((_APmav_land *)This)->update();
			return NULL;
		}

	protected:
		_DistSensorBase* m_pDS = nullptr;
		Vector2f m_vDSrange = Vector2f::Zero();

		vector<AP_LAND_TAG> m_vTags;
		AP_LAND_TAG* m_pTag = nullptr;
		Vector2f m_vFov = Vector2f::Zero(); // cam FOV horiz/vert

		Vector4f m_vComplete = Vector4f::Zero(); // complete condition for vPtarget
		float m_zrK = 1.0;
	};
}
#endif

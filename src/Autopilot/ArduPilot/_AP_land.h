#ifndef OpenKAI_src_Autopilot_AP__AP_land_H_
#define OpenKAI_src_Autopilot_AP__AP_land_H_

#include "_AP_follow.h"
#include "../../Detector/_DetectorBase.h"
#include "../../Utility/utilEvent.h"

namespace kai
{
	struct AP_LAND_TAG
	{
		int m_id = -1;
		int m_priority = 0;
		vFloat2 m_vSize = {0, FLT_MAX}; // complete once bigger than .y

		float m_s = 0.0;
		float m_K = 1.0;

		void updateK(float s)
		{
			m_s = s;
			m_K = (m_vSize.constrain(s) - m_vSize.x) / m_vSize.len();
		}

		bool b(void)
		{
			return (m_s > m_vSize.y);
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
		virtual void console(void *pConsole);
		virtual void draw(void *pFrame);

		bool bComplete(void);

	protected:
		virtual AP_LAND_TAG* getTag(int id);
		virtual bool updateTarget(void);
		virtual AP_LAND_TAG* findTag(void);
		static void *getUpdate(void *This)
		{
			((_AP_land *)This)->update();
			return NULL;
		}

	public:
		float m_zrK;
		float m_rAlt;
		float m_rAltComplete;
		float m_rTargetComplete;
		bool m_bRtargetComplete;

		INTERVAL_EVENT m_ieHdgCmd;

		vector<AP_LAND_TAG> m_vTags;
		AP_LAND_TAG* m_pTag;

	};
}
#endif

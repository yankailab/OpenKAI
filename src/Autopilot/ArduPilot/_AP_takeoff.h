#ifndef OpenKAI_src_Autopilot_AP__AP_takeoff_H_
#define OpenKAI_src_Autopilot_AP__AP_takeoff_H_

#include "_AP_base.h"

namespace kai
{

	class _AP_takeoff : public _StateBase
	{
	public:
		_AP_takeoff();
		~_AP_takeoff();

		bool init(void *pKiss);
		bool start(void);
		int check(void);
		void update(void);
		void console(void *pConsole);

	private:
		void updateMission(void);
		static void *getUpdate(void *This)
		{
			((_AP_takeoff *)This)->update();
			return NULL;
		}

	public:
		_AP_base *m_pAP;
		uint32_t m_apMode;
		float m_dAlt;
	};

}
#endif

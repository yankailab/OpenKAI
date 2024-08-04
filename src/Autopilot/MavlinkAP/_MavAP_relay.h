
#ifndef OpenKAI_src_Autopilot_MavAP__MavAP_relay_H_
#define OpenKAI_src_Autopilot_MavAP__MavAP_relay_H_

#include "_MavAP_base.h"

namespace kai
{

	struct AP_relay
	{
		uint8_t m_iChan;
		bool m_bRelay;

		void init(void)
		{
			m_iChan = 9;
			m_bRelay = false;
		}
	};

	class _MavAP_relay : public _ModuleBase
	{
	public:
		_MavAP_relay();
		~_MavAP_relay();

		int init(void *pKiss);
		int start(void);
		int check(void);
		void update(void);
		void console(void *pConsole);

	private:
		void updateRelay(void);
		static void *getUpdate(void *This)
		{
			((_MavAP_relay *)This)->update();
			return NULL;
		}

	private:
		_MavAP_base *m_pAP;
		vector<AP_relay> m_vRelay;
	};

}

#endif

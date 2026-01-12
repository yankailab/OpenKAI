
#ifndef OpenKAI_src_Autopilot_APmavlink__APmavlink_relay_H_
#define OpenKAI_src_Autopilot_APmavlink__APmavlink_relay_H_

#include "_APmavlink_base.h"

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

	class _APmavlink_relay : public _ModuleBase
	{
	public:
		_APmavlink_relay();
		~_APmavlink_relay();

		bool init(const json& j);
		bool start(void);
		bool check(void);
		void update(void);
		void console(void *pConsole);

	private:
		void updateRelay(void);
		static void *getUpdate(void *This)
		{
			((_APmavlink_relay *)This)->update();
			return NULL;
		}

	private:
		_APmavlink_base *m_pAP;
		vector<AP_relay> m_vRelay;
	};

}

#endif

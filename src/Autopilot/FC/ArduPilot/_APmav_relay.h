
#ifndef OpenKAI_src_Autopilot_FC_ArduPilot__APmav_relay_H_
#define OpenKAI_src_Autopilot_FC_ArduPilot__APmav_relay_H_

#include "_APmav_base.h"

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

	class _APmav_relay : public _ModuleBase
	{
	public:
		_APmav_relay();
		~_APmav_relay();

		virtual bool loadConfig(void) override;
		virtual bool link(void) override;
		virtual bool start(void);
		virtual bool check(void);
		virtual void update(void);
		virtual void console(void *pConsole);

	private:
		void updateRelay(void);
		static void *getUpdate(void *This)
		{
			((_APmav_relay *)This)->update();
			return NULL;
		}

	protected:
		_APmav_base *m_pAP = nullptr;
		vector<AP_relay> m_vRelay;
	};

}

#endif

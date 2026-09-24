#ifndef OpenKAI_src_Autopilot_ArduPilot__APmav_rcChannel_H_
#define OpenKAI_src_Autopilot_ArduPilot__APmav_rcChannel_H_

#include "_APmav_base.h"
#include "../../Utility/RC.h"

namespace kai
{

	class _APmav_rcChannel : public _ModuleBase
	{
	public:
		_APmav_rcChannel();
		~_APmav_rcChannel();

		virtual bool init(const json& j);
		virtual bool link(const json& j, ModuleMgr* pM);
		virtual bool start(void);
		virtual bool check(void);
		virtual void update(void);
		virtual void console(void *pConsole);

	private:
		void updateRCchannel(void);
		static void *getUpdate(void *This)
		{
			((_APmav_rcChannel *)This)->update();
			return NULL;
		}

	protected:
		_APmav_base *m_pAP = nullptr;
		RC_CHANNEL m_rcMode;
		RC_CHANNEL m_rcStickV;
		RC_CHANNEL m_rcStickH;
	};

}

#endif

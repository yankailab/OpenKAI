#ifndef OpenKAI_src_Autopilot_APmavlink__APmavlink_rcChannel_H_
#define OpenKAI_src_Autopilot_APmavlink__APmavlink_rcChannel_H_

#include "_APmavlink_base.h"
#include "../../Utility/RC.h"

namespace kai
{

	class _APmavlink_rcChannel : public _ModuleBase
	{
	public:
		_APmavlink_rcChannel();
		~_APmavlink_rcChannel();

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
			((_APmavlink_rcChannel *)This)->update();
			return NULL;
		}

	protected:
		_APmavlink_base *m_pAP;
		RC_CHANNEL m_rcMode;
		RC_CHANNEL m_rcStickV;
		RC_CHANNEL m_rcStickH;
	};

}

#endif

#ifndef OpenKAI_src_Autopilot_APmavlink__APmavlink_httpJson_H_
#define OpenKAI_src_Autopilot_APmavlink__APmavlink_httpJson_H_

#include "../../Module/JSON.h"
#include "../../Net/HttpClient.h"
#include "_APmavlink_base.h"

namespace kai
{

	class _APmavlink_httpJson : public _ModuleBase
	{
	public:
		_APmavlink_httpJson();
		~_APmavlink_httpJson();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual void update(void);
		virtual void console(void *pConsole);
		virtual int check(void);

	private:
		void updateHttpSend(void);
		static void *getUpdate(void *This)
		{
			((_APmavlink_httpJson *)This)->update();
			return NULL;
		}

	protected:
		_APmavlink_base *m_pAP;

		HttpClient m_httpC;
		string m_url;
	};

}
#endif

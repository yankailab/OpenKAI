#ifndef OpenKAI_src_Autopilot_AP__AP_httpJson_H_
#define OpenKAI_src_Autopilot_AP__AP_httpJson_H_

#include "../../Module/JSON.h"
#include "../../Net/HttpClient.h"
#include "_AP_base.h"

namespace kai
{

	class _AP_httpJson : public _ModuleBase
	{
	public:
		_AP_httpJson();
		~_AP_httpJson();

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
			((_AP_httpJson *)This)->update();
			return NULL;
		}

	protected:
		_AP_base *m_pAP;

		HttpClient m_httpC;
		string m_url;
	};

}
#endif

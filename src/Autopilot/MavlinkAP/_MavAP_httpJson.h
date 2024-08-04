#ifndef OpenKAI_src_Autopilot_MavAP__MavAP_httpJson_H_
#define OpenKAI_src_Autopilot_MavAP__MavAP_httpJson_H_

#include "../../Module/JSON.h"
#include "../../Net/HttpClient.h"
#include "_MavAP_base.h"

namespace kai
{

	class _MavAP_httpJson : public _ModuleBase
	{
	public:
		_MavAP_httpJson();
		~_MavAP_httpJson();

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
			((_MavAP_httpJson *)This)->update();
			return NULL;
		}

	protected:
		_MavAP_base *m_pAP;

		HttpClient m_httpC;
		string m_url;
	};

}
#endif

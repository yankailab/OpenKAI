#ifndef OpenKAI_src_Autopilot_ArduPilot__APmav_httpJson_H_
#define OpenKAI_src_Autopilot_ArduPilot__APmav_httpJson_H_

#include "../../Net/HttpClient.h"
#include "_APmav_base.h"

namespace kai
{

	class _APmav_httpJson : public _ModuleBase
	{
	public:
		_APmav_httpJson();
		~_APmav_httpJson();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual void update(void);
		virtual void console(void *pConsole);
		virtual bool check(void);

	private:
		void updateHttpSend(void);
		static void *getUpdate(void *This)
		{
			((_APmav_httpJson *)This)->update();
			return NULL;
		}

	protected:
		_APmav_base *m_pAP = nullptr;

		HttpClient m_httpC;
		string m_url = "";
	};

}
#endif

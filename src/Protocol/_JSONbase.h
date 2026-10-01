#ifndef OpenKAI_src_Protocol__JSONbase_H_
#define OpenKAI_src_Protocol__JSONbase_H_

#include "_ProtocolBase.h"
#include "../UI/_Console.h"
#include <openssl/md5.h>

namespace kai
{

	class _JSONbase : public _ProtocolBase
	{
	public:
		_JSONbase();
		~_JSONbase();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(InstanceMgr *pM) override;
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);

		virtual bool sendJson(const json &j);

	protected:
		virtual void send(void);
		virtual void sendHeartbeat(void);

		virtual bool recvJson(string *pStr);
		virtual void handleJson(const string &str);
		virtual void md5(const string &str, string *pDigest);
		virtual bool str2JSON(const string &str, json &j);

	private:
		void updateW(void);
		static void *getUpdateW(void *This)
		{
			((_JSONbase *)This)->updateW();
			return NULL;
		}

		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_JSONbase *)This)->updateR();
			return NULL;
		}

	protected:
		string m_msgFinishSend = "";
		string m_msgFinishRecv = "EOJ";

		INTERVAL_EVENT m_ieSendHB;
	};

}
#endif

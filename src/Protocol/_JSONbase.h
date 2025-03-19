#ifndef OpenKAI_src_Protocol__JSONbase_H_
#define OpenKAI_src_Protocol__JSONbase_H_

#include "_ProtocolBase.h"
#include "../IO/_IObase.h"
#include "../UI/_Console.h"
#include <openssl/md5.h>

using namespace picojson;

#define JB_N_BUF 512

namespace kai
{

	class _JSONbase : public _ProtocolBase
	{
	public:
		_JSONbase();
		~_JSONbase();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

	protected:
		virtual void send(void);
		virtual void sendHeartbeat(void);
		virtual bool sendJson(picojson::object &o);

		virtual bool recvJson(string *pStr);
		virtual void handleJson(const string &str);
		virtual void md5(const string &str, string *pDigest);
		virtual bool str2JSON(const string &str, picojson::value *pJson);

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
		string m_msgFinishSend;
		string m_msgFinishRecv;

		INTERVAL_EVENT m_ieSendHB;
	};

}
#endif

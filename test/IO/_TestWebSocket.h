#ifndef OpenKAI_test__TestWebSocket_H_
#define OpenKAI_test__TestWebSocket_H_

#include "../_TestBase.h"
#include "../../src/IO/_WebSocketServer.h"

namespace kai
{

	class _TestWebSocket : public _TestBase
	{
	public:
		_TestWebSocket();
		~_TestWebSocket();

		virtual bool init(const json& j);
		virtual bool link(const json& j, ModuleMgr* pM);
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);

	protected:
		void update(void);
		static void *getUpdate(void *This)
		{
			((_TestWebSocket *)This)->update();
			return NULL;
		}

	protected:
		_WebSocketServer* m_pWSserver;
	};

}
#endif

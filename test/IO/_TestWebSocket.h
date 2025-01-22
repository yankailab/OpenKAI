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

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);
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

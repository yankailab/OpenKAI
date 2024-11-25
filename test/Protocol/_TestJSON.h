#ifndef OpenKAI_test__TestJSON_H_
#define OpenKAI_test__TestJSON_H_

#include "../_TestBase.h"
#include "../../src/Protocol/_JSONbase.h"

namespace kai
{

	class _TestJSON : public _TestBase
	{
	public:
		_TestJSON();
		~_TestJSON();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

	protected:
		void update(void);
		static void *getUpdate(void *This)
		{
			((_TestJSON *)This)->update();
			return NULL;
		}

	protected:
		_JSONbase* m_pJsender;
		_JSONbase* m_pJreceiver;
	};

}
#endif

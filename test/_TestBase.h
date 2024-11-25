#ifndef OpenKAI_test__TestBase_H_
#define OpenKAI_test__TestBase_H_

#include "../src/Base/_ModuleBase.h"
#include "../src/UI/_Console.h"

namespace kai
{

	class _TestBase : public _ModuleBase
	{
	public:
		_TestBase();
		~_TestBase();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

	protected:
		void update(void);
		static void *getUpdate(void *This)
		{
			((_TestBase *)This)->update();
			return NULL;
		}

	protected:

	};

}
#endif

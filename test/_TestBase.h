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

		virtual bool init(const json& j);
		virtual bool link(const json& j, ModuleMgr* pM);
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);

	protected:
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_TestBase *)This)->update();
			return NULL;
		}

	protected:

	};

}
#endif

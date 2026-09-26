#ifndef OpenKAI_src_Swarm__SwarmCtrl_H_
#define OpenKAI_src_Swarm__SwarmCtrl_H_

#include "_SwarmBase.h"

namespace kai
{

	class _SwarmCtrl : public _ModuleBase
	{
	public:
		_SwarmCtrl();
		~_SwarmCtrl();

		virtual bool loadConfig(void) override;
		virtual bool link(void) override;
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);

	private:
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_SwarmCtrl *)This)->update();
			return NULL;
		}

	protected:
	};

}
#endif

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

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
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

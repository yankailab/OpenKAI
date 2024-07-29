#ifndef OpenKAI_src_State_StateBase_H_
#define OpenKAI_src_State_StateBase_H_

#include "../Base/_ModuleBase.h"

namespace kai
{

	enum STATE_TYPE
	{
		state_unknown,
		state_base,
		state_land,
		state_loiter,
		state_goto,
		state_rth,
		state_wp,
		state_takeoff,
	};

	class StateBase : public BASE
	{
	public:
		StateBase();
		~StateBase();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual void console(void *pConsole);

		virtual void reset(void);
		virtual void update(void);
		virtual bool bComplete(void);
		virtual string getNext(void);
		virtual STATE_TYPE getType(void);

		virtual void updateModules(void);

	protected:
		STATE_TYPE m_type;
		string m_next;
		uint64_t m_tStart;
		uint64_t m_tStamp;
		uint64_t m_tTimeout;
		bool m_bComplete;

		vector<_ModuleBase *> m_vpModulePause;
		vector<_ModuleBase *> m_vpModuleResume;

	};

}
#endif

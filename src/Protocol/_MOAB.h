#ifndef OpenKAI_src_Protocol__MOAB_H_
#define OpenKAI_src_Protocol__MOAB_H_

#include "../IO/_IOBase.h"

namespace kai
{

	class _MOAB : public _ModuleBase
	{
	public:
		_MOAB();
		~_MOAB();

		bool init(void *pKiss);
		bool start(void);
		void console(void *pConsole);

		void setSpeed(float speed, float steer);

	private:
		void update(void);
		void send(void);
		static void *getUpdate(void *This)
		{
			((_MOAB *)This)->update();
			return NULL;
		}

	public:
		_IOBase *m_pIO;

		vFloat2 m_vK;
		vFloat2 m_vSpeed;
	};

}
#endif

#ifndef OpenKAI_src_Actuator__DDSM_H_
#define OpenKAI_src_Actuator__DDSM_H_

#include "../_ActuatorBase.h"
#include "../../IO/_IObase.h"
#include "../../Utility/util.h"

#define DDSM_CMD_NB 10

namespace kai
{

	class _DDSM : public _ActuatorBase
	{
	public:
		_DDSM();
		~_DDSM();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

	protected:
		bool setMode(void);
		bool readCMD(void);

	private:
		void sendCMD(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_DDSM *)This)->update();
			return NULL;
		}

		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_DDSM *)This)->updateR();
			return NULL;
		}

	public:
		_Thread *m_pTr;
		_IObase *m_pIO;
		uint8_t m_iMode;
	};

}
#endif

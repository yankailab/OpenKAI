#ifndef OpenKAI_src_Actuator__DDSM_H_
#define OpenKAI_src_Actuator__DDSM_H_

#include "../_ActuatorBase.h"
#include "../../IO/_IObase.h"
#include "../../Utility/util.h"

#define DDSM_CMD_NB 10

namespace kai
{
	enum DDSM_MODE
	{
		ddsm_current = 0x01,
		ddsm_speed = 0x02,
		ddsm_pos = 0x03,
	};

	class _DDSM : public _ActuatorBase
	{
	public:
		_DDSM();
		~_DDSM();

		virtual bool init(const json& j);
		virtual bool link(const json& j, ModuleMgr* pM);
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);

	protected:
		bool setID(void);
		bool setMode(void);
		bool setOutput(int16_t v);
		bool readCMD(void);
		virtual void update(void);
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

	protected:
		_Thread *m_pTr;
		_IObase *m_pIO;

		DDSM_MODE m_ddsmMode;
	};

}
#endif

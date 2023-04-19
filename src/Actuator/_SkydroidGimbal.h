#ifndef OpenKAI_src_Actuator__SkydroidGimbal_H_
#define OpenKAI_src_Actuator__SkydroidGimbal_H_

#include "_ActuatorBase.h"
#include "../IO/_IOBase.h"

namespace kai
{
	class _SkydroidGimbal : public _ActuatorBase
	{
	public:
		_SkydroidGimbal();
		~_SkydroidGimbal();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual void console(void *pConsole);

		virtual bool readCMD(void);
		virtual void handleCMD(void);

	private:
		void sendCMD(void);
		char calcCRC(const char *cmd, uint8_t len);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_SkydroidGimbal *)This)->update();
			return NULL;
		}

		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_SkydroidGimbal *)This)->updateR();
			return NULL;
		}

	public:
		_Thread *m_pTr;
		_IOBase *m_pIO;

		uint8_t m_pB[64];
		string m_cmd; // for quick test
	};

}
#endif

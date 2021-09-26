#ifndef OpenKAI_src_Protocol__SBus_H_
#define OpenKAI_src_Protocol__SBus_H_

#include "../IO/_IOBase.h"

//SBus definition
#define HEADER_ 0x0F
#define FOOTER_ 0x00
#define FOOTER2_ 0x04
#define LEN_ 35
#define CH17_ 0x01
#define CH18_ 0x02
#define LOST_FRAME_ 0x04
#define FAILSAFE_ 0x08

namespace kai
{

	class _SBus : public _ModuleBase
	{
	public:
		_SBus();
		~_SBus();

		bool init(void *pKiss);
		bool start(void);
		void console(void *pConsole);

	private:
		void send(void);
		void updateW(void);
		static void *getUpdateW(void *This)
		{
			((_SBus *)This)->updateW();
			return NULL;
		}

		bool recv(void);
		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_SBus *)This)->updateR();
			return NULL;
		}

		void encode(void);
		void decode(void);
		uint8_t checksum(uint8_t *pB, uint8_t n);
		void reset(void);

	public:
		_Thread *m_pTr;
		_IOBase *m_pIO;

		uint8_t m_pB[LEN_];
		uint8_t m_iB;
		uint8_t m_flag;
		bool m_bLostFrame;
		bool m_bFailSafe;
		bool m_bCh17;
		bool m_bCh18;
		uint16_t m_pC[16];
	};

}
#endif

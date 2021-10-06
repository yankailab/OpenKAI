#ifndef OpenKAI_src_Protocol__SBus_H_
#define OpenKAI_src_Protocol__SBus_H_

#include "../IO/_IOBase.h"
#include "../Utility/RC.h"

//SBus definition
#define SBUS_HEADER_ 0x0F
#define SBUS_FOOTER_ 0x00
#define SBUS_FOOTER2_ 0x04
#define SBUS_CH17_ 0x01
#define SBUS_CH18_ 0x02
#define SBUS_LOST_FRAME_ 0x04
#define SBUS_FAILSAFE_ 0x08
#define SBUS_NCHAN 16

#define SBUS_NBUF 35

namespace kai
{
	class _SBus : public _ModuleBase
	{
	public:
		_SBus();
		~_SBus();

		bool init(void *pKiss);
		bool link(void);
		bool start(void);
		int check(void);
		void console(void *pConsole);

		uint16_t raw(int i);
		float v(int i);

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

		void encodeRaw(void);
		void decodeRaw(void);
		void encode(void);
		void decode(void);
		uint8_t checksum(uint8_t *pB, uint8_t n);
		void reset(void);

	public:
		_Thread *m_pTr;
		_IOBase *m_pIO;
		bool m_bSend;

		bool m_bRawSbus;
		int m_nFrame;

		uint8_t m_pB[SBUS_NBUF];
		uint8_t m_iB;
		uint8_t m_flag;
		bool m_bLostFrame;
		bool m_bFailSafe;
		bool m_bCh17;
		bool m_bCh18;
		RC_CHANNEL m_pRC[SBUS_NCHAN];
		vInt3 m_vRawRC;		
	};

}
#endif

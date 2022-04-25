#ifndef OpenKAI_src_Protocol__PWMio_H_
#define OpenKAI_src_Protocol__PWMio_H_

#include "_ProtocolBase.h"
#include "../Utility/RC.h"

#define ARDU_CMD_PWM 0
#define ARDU_CMD_HB 1

#define PWMIO_N_CHAN 16

namespace kai
{

class _PWMio: public _ProtocolBase
{
public:
	_PWMio();
	~_PWMio();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual void console(void* pConsole);

	virtual void set(int iChan, uint16_t v);
	virtual uint16_t getRaw(int iChan);

private:
	void send(void);
	void handleCMD(void);
	void updateW(void);
	static void* getUpdateW(void* This)
	{
		((_PWMio *) This)->updateW();
		return NULL;
	}

	void updateR(void);
	static void* getUpdateR(void* This)
	{
		((_PWMio *) This)->updateR();
		return NULL;
	}

public:
	uint8_t m_nCr;
	RC_CHANNEL m_pCr[PWMIO_N_CHAN];

	uint8_t m_nCw;
	RC_CHANNEL m_pCw[PWMIO_N_CHAN];

};

}
#endif

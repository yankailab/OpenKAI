#ifndef OpenKAI_src_Protocol__USR_CANET_H_
#define OpenKAI_src_Protocol__USR_CANET_H_

#include "_CANbase.h"
#include "../IO/_IObase.h"
#include "../Utility/util.h"

#define CANET_BUF_N 13

namespace kai
{

	class _USR_CANET : public _CANbase
	{
	public:
		_USR_CANET();
		~_USR_CANET();

		virtual bool init(const json& j);
		virtual bool link(const json& j, ModuleMgr* pM);
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);

		virtual bool open(void);
		virtual bool bOpen(void);
		virtual void close(void);

		virtual bool sendFrame(const CAN_F &f);
		virtual bool readFrame(CAN_F *pF);
		virtual void handleFrame(const CAN_F &f);

	private:
		void update(void);
		static void *getUpdate(void *This)
		{
			((_USR_CANET *)This)->update();
			return NULL;
		}

	protected:
		_IObase *m_pIO;
	};

}
#endif

#ifndef OpenKAI_src_Protocol__SocketCAN_H_
#define OpenKAI_src_Protocol__SocketCAN_H_

#include <linux/can.h>
#include <linux/can/raw.h>
#include "_CANbase.h"

namespace kai
{
	class _SocketCAN : public _CANbase
	{
	public:
		_SocketCAN();
		~_SocketCAN();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

		virtual bool open(void);
		virtual void close(void);

		virtual bool sendFrame(const CAN_F& f);
		virtual bool readFrame(CAN_F* pF);
		virtual void handleFrame(const CAN_F &f);

	private:
		void update(void);
		static void *getUpdate(void *This)
		{
			((_SocketCAN *)This)->update();
			return NULL;
		}

	protected:
		string m_ifName;
		int m_socket;
	};

}
#endif

#ifndef OpenKAI_src_Actuator__FTservo_H_
#define OpenKAI_src_Actuator__FTservo_H_

#include "_ActuatorBase.h"
#include <SMS_STS.h>

namespace kai
{

	// SMS/STS protocol servos; command values use native SDK units.
	class _FTservo : public _ActuatorBase
	{
	public:
		_FTservo();
		~_FTservo() override;

		bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		bool start(void) override;
		void stop(void) override;
		bool check(void) override;
		void console(void *pConsole) override;

	protected:
		bool open(void);
		void close(void);
		bool commandOK(int result);
		bool readStatus(void);
		bool updateMove(void);
		bool stopMove(void);
		void update(void) override;
		static void *getUpdate(void *pThis)
		{
			static_cast<_FTservo *>(pThis)->update();
			return nullptr;
		}

		// Keep upstream unchanged: close the actual descriptor and bound reads
		// on disconnect (the upstream end/read implementations do not).
		class Serial : public SMS_STS
		{
		public:
			void disconnect(void);

		protected:
			int readSCS(unsigned char *pData, int nBytes) override;
			int readSCS(unsigned char *pData, int nBytes, unsigned long timeoutMs) override;
		};

		string m_port;
		int m_baud = 115200;
		bool m_bOpened = false;
		bool m_bTorque = false;
		bool m_bMoveSent = false;
		int m_activeMode = -1;
		int m_status = 0;
		int m_error = 0;
		Serial m_servo;
	};

}
#endif

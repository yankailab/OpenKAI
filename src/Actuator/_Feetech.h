/*
 * _Feetech.h
 *
 *  Created on: Jan 6, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_Actuator__Feetech_H_
#define OpenKAI_src_Actuator__Feetech_H_

#include "_ActuatorBase.h"
#include "../Dependency/Feetech/SCServo.h"

namespace kai
{

	class _Feetech : public _ActuatorBase
	{
	public:
		_Feetech();
		~_Feetech();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual int check(void);
		virtual bool start(void);
		virtual void console(void *pConsole);

	protected:
		bool open(void);
		void close(void);

		bool setID(uint16_t iSlave);
		bool setBaudrate(uint32_t baudrate);
		bool saveData(void);

		bool setPos(void);
		bool setSpeed(void);
		bool setAccel(void);

		bool readStatus(void);
		void updateMove(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_Feetech *)This)->update();
			return NULL;
		}

	public:
		string m_port;
		int m_baud;
		bool m_bOpen;
		SMSBL m_servo;
		uint8_t m_ID;
		ACTUATOR_AXIS *m_pA;

		INTERVAL_EVENT m_ieReadStatus;
	};

}
#endif

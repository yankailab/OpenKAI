/*
 * _FeeTech.h
 *
 *  Created on: Jan 6, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_Actuator__FeeTech_H_
#define OpenKAI_src_Actuator__FeeTech_H_

#include "_ActuatorBase.h"
#include "../IO/_IObase.h"

#define FEETECH_HEADER 6
// 0xff 0xff ID Length Instruction params.. Checksum

namespace kai
{

	struct FEETECH_ADDR
	{
		// EEPROM
		uint8_t m_ID = 5;
		uint8_t m_baudrate = 6;
		uint8_t m_angleMin = 9;
		uint8_t m_angleMax = 11;

		// RAM
		uint8_t m_bTorque = 40;
		uint8_t m_accel = 41;
		uint8_t m_posTarget = 42;
		uint8_t m_spdTarget = 46;

		uint8_t m_bLock = 55;
		uint8_t m_pos = 56;
		uint8_t m_spd = 58;
		uint8_t m_load = 60;
		uint8_t m_voltage = 62;
		uint8_t m_temprature = 63;
		uint8_t m_current = 69;

	};

	class _FeeTech : public _ActuatorBase
	{
	public:
		_FeeTech();
		~_FeeTech();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual int check(void);
		virtual bool start(void);
		virtual void console(void *pConsole);

	protected:
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
			((_FeeTech *)This)->update();
			return NULL;
		}

	public:
		_IObase *m_pIO;
		uint8_t m_ID;
		ACTUATOR_AXIS *m_pA;

		FEETECH_ADDR m_addr;
		INTERVAL_EVENT m_ieReadStatus;
	};

}
#endif

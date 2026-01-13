/*
 * _BenewakeTF.h
 *
 *  Created on: July 24, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_Sensor__BenewakeTF_H_
#define OpenKAI_src_Sensor__BenewakeTF_H_

#include "_DistSensorBase.h"
#include "../../IO/_IObase.h"

#define BENEWAKE_BEGIN 0x59

namespace kai
{

	struct BENEWAKE_FRAME
	{
		uint8_t m_header;
		int m_iB;
		uint8_t *m_pB;

		void init(int nB)
		{
			reset();

			if (nB <= 0)
				nB = 256;
			m_pB = new uint8_t[nB];
		}

		void reset(void)
		{
			m_header = 0;
			m_iB = 0;
		}
	};

	class _BenewakeTF : public _DistSensorBase
	{
	public:
		_BenewakeTF();
		~_BenewakeTF();

		virtual bool init(const json& j);
		virtual bool start(void);
		virtual void console(void *pConsole);

		DIST_SENSOR_TYPE type(void);
		uint8_t verifyCheckSum(const uint8_t *data, size_t dataLength);

	private:
		bool readCMD(void);
		void handleCMD(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_BenewakeTF *)This)->update();
			return NULL;
		}

	public:
		_IObase *m_pIO;
		BENEWAKE_FRAME m_frame;
	};

}
#endif

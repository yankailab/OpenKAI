/*
 * _BenewakeTF.h
 *
 *  Created on: July 24, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_Sensor__BenewakeTF_H_
#define OpenKAI_src_Sensor__BenewakeTF_H_

#include "../../Base/_ModuleBase.h"
#include "../../DataObject/PCLframe.h"
#include "../../DataObject/BytePacketStream.h"

#define BENEWAKE_BEGIN 0x59

namespace kai
{

	struct BENEWAKE_FRAME
	{
		uint8_t m_header;
		int m_iB;
		uint8_t *m_pB = nullptr;

		void init(int nB)
		{
			DEL_ARRAY(m_pB);
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

	class _BenewakeTF : public _ModuleBase
	{
	public:
		_BenewakeTF();
		~_BenewakeTF();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(InstanceMgr *pM) override;
		virtual bool start(void);
		virtual void console(void *pConsole);

		uint8_t verifyCheckSum(const uint8_t *data, size_t dataLength);

	private:
		bool readByte(uint8_t *pB);
		bool readCMD(void);
		void handleCMD(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_BenewakeTF *)This)->update();
			return NULL;
		}

	protected:
		PCLframe *m_pPCLout = nullptr;
		Vector2f m_vRange = Vector2f(0, FLT_MAX);

		BytePacketStream *m_pBpStreamIn = nullptr;
		uint64_t m_tLastBpStreamIn = 0;
		vector<uint8_t> m_vBytesIn;
		size_t m_iByteIn = 0;
		BENEWAKE_FRAME m_frame;
	};

}
#endif

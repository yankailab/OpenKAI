/*
 * BytePacketStream.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src_DataObject_BytePacketStream_H_
#define OpenKAI_src_DataObject_BytePacketStream_H_

#include "DataObjStream.h"

namespace kai
{

	struct BYTE_PACKET
	{
		vector<uint8_t> m_vB;
		uint64_t m_tStamp = 0;

		bool clear(size_t nB = 0)
		{
			m_vB.clear();
			m_tStamp = 0;

			if (nB > 0)
			{
				m_vB.reserve(nB);
			}

			return true;
		}

		void set(const vector<uint8_t> &vB, uint64_t tStamp = 0)
		{
			m_vB = vB;
			updateTstamp(tStamp);
		}

		const vector<uint8_t> &get(void)
		{
			return m_vB;
		}

		void updateTstamp(uint64_t tStamp = 0)
		{
			if (tStamp == 0)
			{
				m_tStamp = getTns();
			}
			else
			{
				m_tStamp = tStamp;
			}
		}

		uint64_t getTstamp(void)
		{
			return m_tStamp;
		}
	};

	class BytePacketStream : public DataObjStream<BYTE_PACKET>
	{
	public:
		BytePacketStream();
		virtual ~BytePacketStream();
		bool loadConfig(void);
		bool saveConfig(bool bExport);

		void console(void *pConsole) override;

		bool clear(size_t nPacket = 0, size_t nPacketBuf = 0);

	protected:
		size_t m_nPacket = 256; // maximum retained packets
		size_t m_nPbuf = 2000;	 // initial byte capacity of each packet
	};

}
#endif

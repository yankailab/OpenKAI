/*
 * BytePacket.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataStream__BytePacket__H_
#define OpenKAI_src__DataStream__BytePacket__H_

#include "DataObjBase.h"

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

		void set(const vector<uint8_t>& vB, uint64_t tStamp = 0)
		{
			m_vB = vB;
			updateTstamp(tStamp);
		}

		const vector<uint8_t>& get(void)
		{
			return m_vB;
		}

		void updateTstamp(uint64_t tStamp = 0)
		{
			if (tStamp == 0)
				m_tStamp = getTns();
			else
				m_tStamp = tStamp;
		}

		uint64_t getTstamp(void)
		{
			return m_tStamp;
		}
	};

	class BytePacket : public DataObjBase
	{
	public:
		BytePacket();
		virtual ~BytePacket();
		bool loadConfig(void);
		bool saveConfig(bool bExport = false);

		void console(void *pConsole) override;

		bool clear(size_t nPacket = 0, size_t nPacketBuf = 0);
		void addPacket(const vector<uint8_t>& vB, uint64_t tStamp = 0);
		void getPackets(vector<uint8_t>& vB, uint64_t tStampFrom = 0);

	protected:
		vector<BYTE_PACKET> m_vPacket;
		int m_nPacket = 0;	// number of packets to be reserved
		int m_nPbuf = 0;	// number of bytes reserved in each packet
		int m_iPset = 0;	// index of packet ring buf written
		std::shared_mutex m_sMutex;
	};

}
#endif

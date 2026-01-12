/*
 * _IObase.h
 *
 *  Created on: June 16, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_IO_IObase_H_
#define OpenKAI_src_IO_IObase_H_

#include "../Base/_ModuleBase.h"
#include "../UI/_Console.h"

#define IO_BUF_N 2000

namespace kai
{

	enum IO_TYPE
	{
		io_none,
		io_serialPort,
		io_file,
		io_tcp,
		io_udp,
		io_webSocket
	};

	enum IO_STATUS
	{
		io_unknown,
		io_closed,
		io_opened
	};

	struct IO_PACKET
	{
		uint8_t *m_pB = NULL;
		int m_nB;
		int m_nBw;

		bool init(int nB)
		{
			m_pB = new uint8_t[nB];
			NULL_F(m_pB);
			m_nB = nB;
			m_nBw = 0;

			return true;
		}

		void release(void)
		{
			DEL(m_pB);
		}

		int set(uint8_t *pB, int nB)
		{
			m_nBw = nB;
			if (m_nBw > m_nB)
				m_nBw = m_nB;

			memcpy(m_pB, pB, m_nBw);

			return m_nBw;
		}
	};

	struct IO_PACKET_FIFO
	{
		IO_PACKET *m_pP = NULL;
		int m_nP;
		int m_iPset;
		int m_iPget;
		pthread_mutex_t m_mutex;

		bool init(int nB, int nP)
		{
			m_pP = new IO_PACKET[nP];
			NULL_F(m_pP);
			m_nP = nP;

			pthread_mutex_init(&m_mutex, NULL);

			int iP;
			for (iP = 0; iP < m_nP; iP++)
			{
				if (!m_pP[iP].init(nB))
					break;
			}

			if (iP < nP)
			{
				release();
				return false;
			}

			clear();
			return true;
		}

		void release(void)
		{
			clear();

			for (int i = 0; i < m_nP; i++)
			{
				m_pP[i].release();
			}

			DEL(m_pP);
			pthread_mutex_destroy(&m_mutex);
		}

		void clear(void)
		{
			m_iPset = 0;
			m_iPget = 0;
		}

		void setPacket(uint8_t *pB, int nB)
		{
			NULL_(pB);

			pthread_mutex_lock(&m_mutex);

			int nBw = 0;
			while (nBw < nB)
			{
				IO_PACKET *pP = &m_pP[m_iPset];
				nBw += pP->set(&pB[nBw], nB - nBw);

				if (m_iPset == m_nP - 1)
					m_iPset = 0;
				else
					m_iPset++;
			}

			pthread_mutex_unlock(&m_mutex);
		}

		int getPacket(uint8_t *pB, int nB)
		{
			IF__(m_iPget == m_iPset, 0);

			IO_PACKET *pP = &m_pP[m_iPget++];
			if (m_iPget >= m_nP)
				m_iPget = 0;

			if (pP->m_nBw > nB)
				return -1;

			memcpy(pB, pP->m_pB, pP->m_nBw);
			return pP->m_nBw;
		}
	};

	class _IObase : public _ModuleBase
	{
	public:
		_IObase();
		virtual ~_IObase();

		virtual bool init(const json& j);
		virtual bool link(const json& j, ModuleMgr* pM);
		virtual void console(void *pConsole);

		virtual IO_TYPE ioType(void);
		virtual bool open(void);
		virtual bool bOpen(void);
		virtual void close(void);

		virtual bool write(uint8_t *pBuf, int nB);
		virtual int read(uint8_t *pBuf, int nB);

		virtual IO_STATUS getIOstatus(void);
		virtual void setIOstatus(IO_STATUS s);

		virtual IO_PACKET_FIFO* getPacketFIFOw(void);

	protected:
		IO_TYPE m_ioType;
		IO_STATUS m_ioStatus;

		IO_PACKET_FIFO m_packetW;
	};

}
#endif

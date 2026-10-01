#include "_USR_CANET.h"

namespace kai
{

	_USR_CANET::_USR_CANET()
	{
		m_nFrameRecv = 0;
	}

	_USR_CANET::~_USR_CANET()
	{
	}

	bool _USR_CANET::loadConfig(void)
	{
		IF_F(!this->_CANbase::loadConfig());

		return true;
	}

	bool _USR_CANET::saveConfig(bool bExport)
	{
		IF_F(!_CANbase::saveConfig(false));

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _USR_CANET::link(InstanceMgr *pM)
	{
		IF_F(!this->_CANbase::link(pM));
		const json &j = *m_pJ;

		string n;
		jKv(j, "BytePacketStreamIn", n);
		m_pBpStreamIn = dynamic_cast<BytePacketStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!n.empty() && !m_pBpStreamIn, "BytePacketStreamIn not found: " + n);

		n.clear();
		jKv(j, "BytePacketStreamOut", n);
		m_pBpStreamOut = dynamic_cast<BytePacketStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!n.empty() && !m_pBpStreamOut, "BytePacketStreamOut not found: " + n);
		IF_F(!m_pBpStreamIn && !m_pBpStreamOut);

		return true;
	}

	bool _USR_CANET::open(void)
	{
		return m_pBpStreamIn || m_pBpStreamOut;
	}

	bool _USR_CANET::bOpen(void)
	{
		return m_pBpStreamIn || m_pBpStreamOut;
	}

	void _USR_CANET::close(void)
	{
		m_iErr = 0;
	}

	bool _USR_CANET::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _USR_CANET::check(void)
	{
		IF_F(!bOpen());

		// use ModuleBase::check() as CANbase uses m_bOpened but CANET does not rely on it
		return this->_ModuleBase::check();
	}

	void _USR_CANET::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();
		}
	}

	bool _USR_CANET::sendFrame(const CAN_F &f)
	{
		IF_F(!check());
		IF_F(f.m_nData > 8);

		uint8_t ctrlB = 0;
		ctrlB |= (f.m_nData & 0x0F);

		if (f.m_bExtended)
			ctrlB |= (1 << 7);

		if (f.m_bRTR)
			ctrlB |= (1 << 6);

		uint8_t pB[CANET_BUF_N];
		memset(pB, 0, CANET_BUF_N);

		// ctrl byte
		pB[0] = ctrlB;
		// ID
		pack_uint32(&pB[1], f.m_ID, true);
		// data
		memcpy(&pB[5], f.m_pData, f.m_nData);

		NULL_F(m_pBpStreamOut);
		m_pBpStreamOut->addPacket(vector<uint8_t>(pB, pB + CANET_BUF_N));

		LOG_I("Sent: id=" + i2str(f.m_ID) + ", len=" + i2str(f.m_nData));
		return true;
	}

	bool _USR_CANET::readFrame(CAN_F *pF)
	{
		IF_F(!check());
		NULL_F(pF);

		NULL_F(m_pBpStreamIn);
		if (m_vFrameBytes.size() < CANET_BUF_N)
		{
			vector<BYTE_PACKET> vPackets;
			m_pBpStreamIn->getPackets(vPackets, m_tLastBpStreamIn);
			for (const BYTE_PACKET &packet : vPackets)
			{
				m_vFrameBytes.insert(m_vFrameBytes.end(), packet.m_vB.begin(), packet.m_vB.end());
				m_tLastBpStreamIn = packet.m_tStamp;
			}
		}
		IF_F(m_vFrameBytes.size() < CANET_BUF_N);

		uint8_t pB[CANET_BUF_N];
		memcpy(pB, m_vFrameBytes.data(), CANET_BUF_N);
		m_vFrameBytes.erase(m_vFrameBytes.begin(), m_vFrameBytes.begin() + CANET_BUF_N);

		pF->clear();
		uint8_t ctrlB = pB[0];
		pF->m_bExtended = (ctrlB & (1 << 7));
		pF->m_bRTR = (ctrlB & (1 << 6));
		pF->m_nData = ctrlB & 0x0F;
		pF->m_ID = *((uint32_t *)&pB[1]);
		memcpy(pF->m_pData, &pB[5], 8);

		LOG_I("Recv: id=" + i2str(pF->m_ID) +
			  ", len=" + i2str(pF->m_nData) +
			  ", data=" + i2str(pF->m_pData[0]) +
			  ", " + i2str(pF->m_pData[1]) +
			  ", " + i2str(pF->m_pData[2]) +
			  ", " + i2str(pF->m_pData[3]) +
			  ", " + i2str(pF->m_pData[4]) +
			  ", " + i2str(pF->m_pData[5]) +
			  ", " + i2str(pF->m_pData[6]) +
			  ", " + i2str(pF->m_pData[7]));

		return true;
	}

	void _USR_CANET::handleFrame(const CAN_F &f)
	{
	}

	void _USR_CANET::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_CANbase::console(pConsole);

		// _Console *pC = (_Console *)pConsole;
	}

}

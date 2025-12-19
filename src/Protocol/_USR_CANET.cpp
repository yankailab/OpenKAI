#include "_USR_CANET.h"

namespace kai
{

	_USR_CANET::_USR_CANET()
	{
		m_pIO = nullptr;
		m_nFrameRecv = 0;
	}

	_USR_CANET::~_USR_CANET()
	{
	}

	int _USR_CANET::init(void *pKiss)
	{
		CHECK_(this->_CANbase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		return OK_OK;
	}

	int _USR_CANET::link(void)
	{
		CHECK_(this->_CANbase::link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		n = "";
		pK->v("_IObase", &n);
		m_pIO = (_IObase *)(pK->findModule(n));
		NULL__(m_pIO, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	bool _USR_CANET::open(void)
	{
		NULL_F(m_pIO);
		return m_pIO->bOpen();
	}

	bool _USR_CANET::bOpen(void)
	{
		NULL_F(m_pIO);
		return m_pIO->bOpen();
	}

	void _USR_CANET::close(void)
	{
		m_iErr = 0;
	}

	int _USR_CANET::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _USR_CANET::check(void)
	{
		NULL__(m_pIO, OK_ERR_NULLPTR);
		IF__(!m_pIO->bOpen(), OK_ERR_NOT_READY);

		// use ModuleBase::check() as CANbase uses m_bOpen but CANET does not rely on it
		return this->_ModuleBase::check();
	}

	void _USR_CANET::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();
		}
	}

	bool _USR_CANET::sendFrame(const CAN_F &f)
	{
		IF_F(check() != OK_OK);
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

		if (!m_pIO->write(pB, CANET_BUF_N))
		{
			LOG_E("m_pIO->write(pB, CANET_BUF_N)");
			m_iErr++;
			return false;
		}

		LOG_I("Sent: id=" + i2str(f.m_ID) + ", len=" + i2str(f.m_nData));
		return true;
	}

	bool _USR_CANET::readFrame(CAN_F *pF)
	{
		IF_F(check() != OK_OK);
		NULL_F(pF);

		uint8_t pB[CANET_BUF_N];
		memset(pB, 0, CANET_BUF_N);

		int nR = m_pIO->read(pB, CANET_BUF_N);
		if (nR <= 0)
		{
			LOG_E("pIO->read(pB, CANET_BUF_N) <= 0");
			m_iErr++;
			return false;
		}

		if(nR < CANET_BUF_N)
		{
			LOG_E("pIO->read(pB, CANET_BUF_N) < CANET_BUF_N");
			m_iErr++;
			return false;
		}

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

		_Console *pC = (_Console *)pConsole;
	}

}

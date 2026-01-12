#include "_CANbase.h"

namespace kai
{

	_CANbase::_CANbase()
	{
		m_bOpen = false;
		m_nFrameRecv = 0;

		m_iErr = 0;
		m_nErrReconnect = 1;
	}

	_CANbase::~_CANbase()
	{
	}

	int _CANbase::init(const json& j)
	{
		CHECK_(this->_ModuleBase::init(j));

		= j.value("nErrReconnect", &m_nErrReconnect);

		return true;
	}

	int _CANbase::link(const json& j, ModuleMgr* pM)
	{
		CHECK_(this->_ModuleBase::link(j, pM));

		string n;

		return true;
	}

	bool _CANbase::open(void)
	{
		return false;
	}

	bool _CANbase::bOpen(void)
	{
		return m_bOpen;
	}

	void _CANbase::close(void)
	{
		m_bOpen = false;
		m_iErr = 0;
	}

	int _CANbase::check(void)
	{
		IF__(!m_bOpen);

		return this->_ModuleBase::check();
	}

	bool _CANbase::sendFrame(const CAN_F& f)
	{
		IF_F(!check());

		return false;
	}

	bool _CANbase::readFrame(CAN_F* pF)
	{
		NULL_F(pF);

		return false;

	}

	void _CANbase::handleFrame(const CAN_F &f)
	{
	}

	void _CANbase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("bOpen = " + i2str(m_bOpen), 1);
		pC->addMsg("nFrameRecv = " + i2str(m_nFrameRecv), 1);
	}

}

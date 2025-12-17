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

	int _CANbase::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("nErrReconnect", &m_nErrReconnect);

		return OK_OK;
	}

	int _CANbase::link(void)
	{
		CHECK_(this->_ModuleBase::link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		return OK_OK;
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
		IF__(!m_bOpen, OK_ERR_NOT_READY);

		return this->_ModuleBase::check();
	}

	bool _CANbase::sendFrame(const CAN_F& f)
	{
		IF_F(check() != OK_OK);

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

#include "_CETCUS.h"

namespace kai
{

_CETCUS::_CETCUS()
{
	m_pIO = NULL;
	m_uavNo = "";
}

_CETCUS::~_CETCUS()
{
}

bool _CETCUS::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	pK->v("uavNo", &m_uavNo);

	string iName;
	iName = "";
	F_ERROR_F(pK->v("_IOBase", &iName));
	m_pIO = (_TCPclient*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pIO,"_IOBase not found");

	return true;
}

bool _CETCUS::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG_E(retCode);
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _CETCUS::check(void)
{
	NULL__(m_pIO, -1);
	IF__(!m_pIO->isOpen(), -1);

	return 0;
}

void _CETCUS::update(void)
{
	while (m_bThreadON)
	{
		if(!m_pIO)
		{
			this->sleepTime(USEC_1SEC);
			continue;
		}

		if(!m_pIO->isOpen())
		{
			if(!m_pIO->open())
			{
				this->sleepTime(USEC_1SEC);
				continue;
			}
		}

		this->autoFPSfrom();

		planeConnect();
		recv();

		this->autoFPSto();
	}
}

bool _CETCUS::recv()
{
	int nB;
	char pB[CETCUS_N_BUF];
	int iB = 0;

	while ((nB = m_pIO->read((unsigned char*)&pB[iB], CETCUS_N_BUF-iB)) > 0)
	{
		iB += nB;
		if(iB >= CETCUS_N_BUF)break;
	}

	string json(pB);

	LOG_I("Received: " + json);
	handleJSON();

	return false;
}

void _CETCUS::handleJSON(void)
{
}

void _CETCUS::planeConnect(void)
{
	IF_(check()<0);

	string test = "{\"action\":\"plane_connect\",\"uav_no\":\"" + m_uavNo + "\"}SWOOLEFN";
	m_pIO->write((unsigned char*)test.c_str(), test.size());

	LOG_I("Sent: " + test);

}

void _CETCUS::draw(void)
{
	this->_ThreadBase::draw();

	string msg = *this->getName();
	if (m_pIO->isOpen())
		msg += ": CONNECTED";
	else
		msg += ": Not connected";

	addMsg(msg);

}

}

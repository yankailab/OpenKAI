#include "_MOAB.h"

namespace kai
{
_MOAB::_MOAB()
{
	m_pIO = NULL;
	m_vK.init(1.0);
	m_vSpeed.init();

}

_MOAB::~_MOAB()
{
}

bool _MOAB::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	pK->v("vK", &m_vK);

	string iName;
	iName = "";
	F_ERROR_F(pK->v("_IOBase", &iName));
	m_pIO = (_IOBase*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pIO,"_IOBase not found");

	return true;
}

bool _MOAB::start(void)
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

void _MOAB::update(void)
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

		send();

		this->autoFPSto();
	}
}

void _MOAB::send(void)
{
	IF_(!m_pIO->isOpen());

	unsigned char pB[16];

	//set each byte of the command
	pB[0] = 0;
	pB[1] = 0;
	pB[2] = 0;
	pB[3] = 0;

	//set the buffer and the byte length to be sent
	m_pIO->write(pB, 4);
}

void _MOAB::setSpeed(float speed, float steer)
{
	speed = constrain(speed, -1.0f, 1.0f);
	steer = constrain(steer, -1.0f, 1.0f);



}

void _MOAB::draw(void)
{
	this->_ThreadBase::draw();

	if (!m_pIO->isOpen())
		addMsg("Not connected");
	else
		addMsg("Connected");

	addMsg("vSpeed=(" + f2str(m_vSpeed.x) + ", " + f2str(m_vSpeed.y) + ")");
}

}

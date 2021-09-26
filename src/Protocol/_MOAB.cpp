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

	bool _MOAB::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vK", &m_vK);

		string n;
		n = "";
		F_ERROR_F(pK->v("_IOBase", &n));
		m_pIO = (_IOBase *)(pK->getInst(n));
		NULL_Fl(m_pIO, "_IOBase not found");

		return true;
	}

	bool _MOAB::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _MOAB::update(void)
	{
		while (m_pT->bRun())
		{
			if (!m_pIO)
			{
				m_pT->sleepT(SEC_2_USEC);
				continue;
			}

			if (!m_pIO->isOpen())
			{
				if (!m_pIO->open())
				{
					m_pT->sleepT(SEC_2_USEC);
					continue;
				}
			}

			m_pT->autoFPSfrom();

			send();

			m_pT->autoFPSto();
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

	void _MOAB::console(void *pConsole)
	{
#ifdef WITH_UI
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		if (!m_pIO->isOpen())
			pC->addMsg("Not connected");
		else
			pC->addMsg("Connected");

		pC->addMsg("vSpeed=(" + f2str(m_vSpeed.x) + ", " + f2str(m_vSpeed.y) + ")");
#endif
	}

}

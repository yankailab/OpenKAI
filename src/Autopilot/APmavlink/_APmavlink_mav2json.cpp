#include "_APmavlink_mav2json.h"

namespace kai
{

	_APmavlink_mav2json::_APmavlink_mav2json()
	{
		m_pAP = nullptr;
	}

	_APmavlink_mav2json::~_APmavlink_mav2json()
	{
	}

	int _APmavlink_mav2json::init(void *pKiss)
	{
		CHECK_(this->_JSONbase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

//        pK->v("dS", &m_dS);

		return OK_OK;
	}

	int _APmavlink_mav2json::link(void)
	{
		CHECK_(this->_JSONbase::link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		n = "";
		pK->v("_APmavlink_base", &n);
		m_pAP = (_APmavlink_base *)(pK->findModule(n));
		NULL__(m_pAP, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	int _APmavlink_mav2json::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		NULL__(m_pTr, OK_ERR_NULLPTR);
		CHECK_(m_pT->start(getUpdateW, this));
		return m_pTr->start(getUpdateR, this);
	}

	int _APmavlink_mav2json::check(void)
	{
		NULL__(m_pAP, OK_ERR_NULLPTR);
		NULL__(m_pAP->m_pMav, OK_ERR_NULLPTR);

		return this->_JSONbase::check();
	}

	void _APmavlink_mav2json::updateW(void)
	{
		while (m_pT->bAlive())
		{
			if (!m_pIO)
			{
				m_pT->sleepT(SEC_2_USEC);
				continue;
			}

			m_pT->autoFPSfrom();

			updateVehicle();

			m_pT->autoFPSto();
		}
	}

	void _APmavlink_mav2json::updateVehicle(void)
	{
		IF_(check() != OK_OK);

		int apMode = m_pAP->getMode();
		vDouble4 vP = m_pAP->getGlobalPos();

		IF_(!m_pAP->bApArmed());
	}

	void _APmavlink_mav2json::send(void)
	{
		IF_(check() != OK_OK);

		if (m_ieSendHB.update(m_pT->getTfrom()))
		{
			//            sendHeartbeat();
		}
	}

	void _APmavlink_mav2json::updateR(void)
	{
		while (m_pTr->bAlive())
		{
			m_pTr->autoFPSfrom();

			if (recv())
			{
				handleMsg(m_strB);
				m_strB.clear();
			}

			m_pTr->autoFPSto();
		}
	}

	void _APmavlink_mav2json::handleMsg(string &str)
	{
		value json;
		IF_(!str2JSON(str, &json));

		object &jo = json.get<object>();
		IF_(!jo["cmd"].is<string>());
		string cmd = jo["cmd"].get<string>();

		if (cmd == "heartbeat")
			heartbeat(jo);
		// else if (cmd == "stat")
		// 	stat(jo);
	}

	void _APmavlink_mav2json::heartbeat(picojson::object &o)
	{
		IF_(check() != OK_OK);
	}

	// void _APmavlink_mav2json::stat(picojson::object &o)
	// {
	// 	IF_(check() != OK_OK);
	// 	IF_(!o["id"].is<double>());
	// 	IF_(!o["stat"].is<string>());

	// 	int vID = o["id"].get<double>();
	// 	string stat = o["stat"].get<string>();
	// }

	void _APmavlink_mav2json::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_JSONbase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
//		pC->addMsg("iMission = " + i2str(m_iMission), 1);

	}

}

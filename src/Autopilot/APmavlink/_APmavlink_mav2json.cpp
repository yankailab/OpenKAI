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
			m_pT->autoFPSfrom();

			update2json();

			m_pT->autoFPSto();
		}
	}

	void _APmavlink_mav2json::update2json(void)
	{
		IF_(check() != OK_OK);

		IF_(!m_ieSendHB.update(m_pT->getTfrom()));

		int apMode = m_pAP->getMode();
		vDouble4 vP = m_pAP->getGlobalPos();
		vFloat3 vA = m_pAP->getAttitude();

// {
// 	"cmd":"vUpdate",	// command name
// 	"vID":0,		// int: vehicle id
// 	"lon":0,		// float: vehicle pos longitude
// 	"lat":0,		// float: vehicle pos latitude
// 	"alt":0,		// float: vehicle pos altitude in meter
// 	"yaw":360,		// float: vehicle attitude yaw (degree), from absolute north (0) to 360 degree
// 	"pitch":0,		// float: vehicle attitude pitch (degree)
// 	"roll":0,		// float: vehicle attitude roll (degree)
// 	"batt":90,		// float: vehicle battery left percentage (0.0 to 100.0)
// 	"hdg":0,		// float: vehicle heading to next waypoint (degree), only if vehicle is in Auto mode, -1 for other modes.
// }

		object r;
		JO(r, "cmd", "vUpdate");
		JO(r, "vID", 0.0);
		JO(r, "lon", vP.y);
		JO(r, "lat", vP.x);
		JO(r, "alt", vP.w);
		JO(r, "yaw", vA.x);
		JO(r, "pitch", vA.y);
		JO(r, "roll", vA.z);
		JO(r, "batt", m_pAP->getBattery());
		JO(r, "hdg", -1.0);
//		sendMsg(r);

        string msg = picojson::value(r).serialize() + m_msgFinishSend;
        _WebSocket *pWS = (_WebSocket *)m_pIO;
        pWS->writeTo(0, (unsigned char *)msg.c_str(), msg.size(), WS_MODE_TXT);

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

	}

}

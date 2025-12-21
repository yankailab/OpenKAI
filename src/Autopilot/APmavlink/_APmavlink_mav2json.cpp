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
		NULL__(m_pAP->getMavlink(), OK_ERR_NULLPTR);

		return this->_JSONbase::check();
	}

	void _APmavlink_mav2json::updateW(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			send();

		}
	}

	void _APmavlink_mav2json::send(void)
	{
		IF_(check() != OK_OK);

		IF_(!m_ieSendHB.updateT(m_pT->getTfrom()));

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
		JO(r, "yaw", vA.x * RAD_2_DEG - 90);
		JO(r, "pitch", vA.y);
		JO(r, "roll", vA.z);
		JO(r, "batt", m_pAP->getBattery());
		JO(r, "hdg", -1.0);
		sendJson(r);
	}

	void _APmavlink_mav2json::updateR(void)
	{
        string strR = "";

        while (m_pTr->bAlive())
        {
            IF_CONT(!recvJson(&strR, m_pIO));

            handleJson(strR);
            strR.clear();
			m_nCMDrecv++;
        }
	}

	void _APmavlink_mav2json::handleJson(const string &str)
	{
		value json;
		IF_(!str2JSON(str, &json));

		object &jo = json.get<object>();
		IF_(!jo["cmd"].is<string>());
		string cmd = jo["cmd"].get<string>();

		if (cmd == "heartbeat")
			handleHeartbeat(jo);
		else if (cmd == "stat")
			handleStat(jo);
	}

	void _APmavlink_mav2json::handleHeartbeat(picojson::object &o)
	{
		IF_(check() != OK_OK);
	}

	void _APmavlink_mav2json::handleStat(picojson::object &o)
	{
		IF_(check() != OK_OK);
		IF_(!o["id"].is<double>());
		IF_(!o["stat"].is<string>());

		int vID = o["id"].get<double>();
		string stat = o["stat"].get<string>();
	}

	void _APmavlink_mav2json::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_JSONbase::console(pConsole);

		_Console *pC = (_Console *)pConsole;

	}

}

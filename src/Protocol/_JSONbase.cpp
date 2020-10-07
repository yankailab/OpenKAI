#include "_JSONbase.h"

namespace kai
{

_JSONbase::_JSONbase()
{
	m_rThreadID = 0;
	m_bRThreadON = false;
	m_pIO = NULL;

	m_msgFinishSend = "SWOOLEFN";
	m_msgFinishRecv = "SWOOLE_SOCKET_FINISH";
}

_JSONbase::~_JSONbase()
{
}

bool _JSONbase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	pK->v("msgFinishSend", &m_msgFinishSend);
	pK->v("msgFinishRecv", &m_msgFinishRecv);

	string n;
	n = "";
	F_ERROR_F(pK->v("_IOBase", &n));
	m_pIO = (_IOBase*) (pK->getInst(n));
	NULL_Fl(m_pIO,"_IOBase not found");

	return true;
}

bool _JSONbase::start(void)
{
	int retCode;

	if(!m_bThreadON)
	{
		m_bThreadON = true;
		retCode = pthread_create(&m_threadID, 0, getUpdateThreadW, this);
		if (retCode != 0)
		{
			LOG_E(retCode);
			m_bThreadON = false;
			return false;
		}
	}

	if(!m_bRThreadON)
	{
		m_bRThreadON = true;
		retCode = pthread_create(&m_rThreadID, 0, getUpdateThreadR, this);
		if (retCode != 0)
		{
			LOG_E(retCode);
			m_bRThreadON = false;
			return false;
		}
	}

	return true;
}

int _JSONbase::check(void)
{
	NULL__(m_pIO, -1);
	IF__(!m_pIO->isOpen(), -1);

	return 0;
}

void _JSONbase::updateW(void)
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

void _JSONbase::send(void)
{
	IF_(check()<0);

	picojson::object o;
	o.insert(make_pair("action", value(string("start_fly"))));

//	picojson::array wpA;
//	for(unsigned int i=0; i< m_vWP.size(); i++)
//	{
//		CETCUS_WAYPOINT* pWP = &m_vWP[i];
//		object wp;
//		wp.insert(make_pair("id", value(i2str(pWP->m_id))));
//		wpA.push_back(value(wp));
//	}
//	o.insert(make_pair("waypoints", value(wpA)));

	string msg = picojson::value(o).serialize() + m_msgFinishSend;
	m_pIO->write((unsigned char*)msg.c_str(), msg.size());
	LOG_I("Sent: " + msg);
}

void _JSONbase::updateR(void)
{
	while (m_bRThreadON)
	{
		recv();
		this->sleepTime(0); //wait for the IObase to wake me up when received data
	}
}

bool _JSONbase::recv()
{
	IF_F(check()<0);

	static string s_strB = "";

	unsigned char B;
	unsigned int nStrFinish = m_msgFinishRecv.length();
	int nB;

	while ((nB = m_pIO->read(&B, 1)) > 0)
	{
		s_strB += B;
		IF_CONT(s_strB.length() <= nStrFinish);

		string lstr = s_strB.substr(s_strB.length()-nStrFinish, nStrFinish);
		IF_CONT(lstr != m_msgFinishRecv);

		s_strB.erase(s_strB.length()-nStrFinish, nStrFinish);
		handleMsg(s_strB);

		LOG_I("Received: " + s_strB);
		s_strB.clear();
	}

	return false;
}

void _JSONbase::handleMsg(string& str)
{
	string error;
	const char* jsonstr = str.c_str();
	picojson::value json;
	picojson::parse(json, jsonstr, jsonstr + strlen(jsonstr), &error);
	IF_(!json.is<object>());

	picojson::object& jo = json.get<picojson::object>();
	string action = jo["action"].get<string>();
//	if(action == "plane_connect");

	int status = (int)jo["status"].get<double>();
	string data = jo["data"].get<string>();
	string message = jo["message"].get<string>();

}

void _JSONbase::draw(void)
{
	this->_ThreadBase::draw();

	string msg = *this->getName();
	if (m_pIO->isOpen())
		msg += ": Connected";
	else
		msg += ": Not connected";

	addMsg(msg);

}

}

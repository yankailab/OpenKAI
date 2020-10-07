#include "_PCui.h"

namespace kai
{

_PCui::_PCui()
{
}

_PCui::~_PCui()
{
}

bool _PCui::init(void* pKiss)
{
	IF_F(!this->_JSONbase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	vector<string> vP;
	pK->a("vPCB", &vP);
	IF_F(vP.empty());
	for(string p : vP)
	{
		_PCtransform* pPCT = (_PCtransform*) (pK->getInst(p));
		IF_CONT(!pPCT);

		m_vPCT.push_back(pPCT);
	}
	IF_F(m_vPCT.empty());

	return true;
}

bool _PCui::start(void)
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

int _PCui::check(void)
{
	return this->_JSONbase::check();
}

void _PCui::updateW(void)
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

void _PCui::send(void)
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

void _PCui::updateR(void)
{
	while (m_bRThreadON)
	{
		recv();
		this->sleepTime(0); //wait for the IObase to wake me up when received data
	}
}

bool _PCui::recv()
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

void _PCui::handleMsg(string& str)
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

//bool _PCui::recv()
//{
//	IF_F(check()<0);
//
//	static string s_strB = "";
//	unsigned char B;
//	int nB;
//
//	while ((nB = m_pIO->read(&B, 1)) > 0)
//	{
//		s_strB += B;
//		IF_CONT(B != ';');
//
//		s_strB.erase(s_strB.length()-1, 1);
//		handleMsg(s_strB);
//
//		LOG_I("Received: " + s_strB);
//		s_strB.clear();
//	}
//
//	return false;
//}
//
//void _PCui::handleMsg(string& str)
//{
//	vector<string> vP = splitBy(str, ',');
//	IF_(vP.size() < 7);
//
//	string sensorName = vP[0];
//	_PCtransform* pPCT = NULL;
//	for(int i=0; i<m_vPCT.size(); i++)
//	{
//		IF_CONT(*m_vPCT[i]->getName() != sensorName);
//
//		pPCT = m_vPCT[i];
//		break;
//	}
//	NULL_(pPCT);
//
//	vFloat3 v;
//	v.init(stof(vP[1]), stof(vP[2]), stof(vP[3]));
//	pPCT->setTranslation(v);
//	v.init(stof(vP[4]), stof(vP[5]), stof(vP[6]));
//	pPCT->setRotation(v);
//}

void _PCui::draw(void)
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

#include "_CETCUS.h"

namespace kai
{

_CETCUS::_CETCUS()
{
	m_rThreadID = 0;
	m_bRThreadON = false;
	m_pIO = NULL;

	m_msgFinishSend = "SWOOLEFN";
	m_msgFinishRecv = "SWOOLE_SOCKET_FINISH";

	m_state = cetcus_connecting;
	m_uavNo = "";
	m_airlineNo = "0";
	m_airlineName = "";
	m_flyID = "";
	m_flyStatus = cetcus_standby;
	m_vPos.init();
	m_vAtti.init();
	m_course = 0.0;
	m_vSpeed.init();
	m_remainOil = 0.0;
	m_remainDist = 0.0;
	m_remainTime = 0.0;
	m_statusMot = cetcus_normal;
	m_statusNav = cetcus_normal;
	m_statusCom = cetcus_normal;

	m_wpStart.init();
	m_wpEnd.init();
}

_CETCUS::~_CETCUS()
{
}

bool _CETCUS::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	pK->v("msgFinishSend", &m_msgFinishSend);
	pK->v("msgFinishRecv", &m_msgFinishRecv);
	pK->v("uavNo", &m_uavNo);

	string iName;
	iName = "";
	F_ERROR_F(pK->v("_IOBase", &iName));
	m_pIO = (_IOBase*) (pK->getInst(iName));
	NULL_Fl(m_pIO,"_IOBase not found");

	return true;
}

bool _CETCUS::start(void)
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

int _CETCUS::check(void)
{
	NULL__(m_pIO, -1);
	IF__(!m_pIO->isOpen(), -1);

	return 0;
}

void _CETCUS::updateW(void)
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

		updateMission();

		this->autoFPSto();
	}
}

void _CETCUS::updateMission(void)
{
	if(m_state == cetcus_connecting)
	{
		planeConnect();
	}
	else if(m_state == cetcus_planeConnected)
	{
		startFly();
	}
	else if(m_state == cetcus_receivedMission)
	{
		startFly();
	}
	else if(m_state == cetcus_startFly)
	{
		detailUpload();
	}
}

void _CETCUS::updateR(void)
{
	while (m_bRThreadON)
	{
		recv();
		this->sleepTime(0);
	}
}

bool _CETCUS::recv()
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

void _CETCUS::handleMsg(string& str)
{
	string error;
	const char* jsonstr = str.c_str();
	picojson::value json;
	picojson::parse(json, jsonstr, jsonstr + strlen(jsonstr), &error);
	IF_(!json.is<object>());

	picojson::object& jo = json.get<picojson::object>();
	string action = jo["action"].get<string>();

	if(action == "plane_connect")
		planeConnect(jo);
	else if(action == "flight_send")
		flightSend(jo);
	else if(action == "start_fly")
		startFly(jo);
	else if(action == "detail_upload")
		detailUpload(jo);
	else if(action == "command")
		command(jo);
	else if(action == "end_fly")
		endFly(jo);
}

void _CETCUS::planeConnect(picojson::object& jo)
{
	int status = (int)jo["status"].get<double>();
	string data = jo["data"].get<string>();
	string message = jo["message"].get<string>();

	if(status != 0 || data != m_uavNo)
	{
		m_state = cetcus_connecting;
		return;
	}

	m_state = cetcus_planeConnected;
	LOG_I("Plane Connected");
}

void _CETCUS::flightSend(picojson::object& jo)
{
	int status = (int)jo["status"].get<double>();
	IF_(status != 0);
	{
		m_state = cetcus_connecting;
		return;
	}

	object d = jo["data"].get<object>();
	m_airlineNo = d["airline_no"].get<string>();
	m_airlineName = d["airline_name"].get<string>();
	m_uavNo = d["uav_no"].get<string>();
	m_wpStart.m_name = d["startpoint_name"].get<string>();
	m_wpStart.m_vPos.y = d["start_point_lon"].get<double>();
	m_wpStart.m_vPos.x = d["start_point_lat"].get<double>();
	m_wpStart.m_vPos.z = d["start_point_alt"].get<double>();
	m_wpEnd.m_name = d["endpoint_name"].get<string>();
	m_wpEnd.m_vPos.y = d["end_point_lon"].get<double>();
	m_wpEnd.m_vPos.x = d["end_point_lat"].get<double>();
	m_wpEnd.m_vPos.z = d["end_point_alt"].get<double>();
	m_startTime = d["start_time"].get<string>();
	m_endTime = d["end_name"].get<string>();


	m_vWP.clear();
	picojson::array& aWP = jo["waypoints"].get<picojson::array>();
	for (const auto& wp : aWP)
	{
		object wo = wp.get<object>();

		CETCUS_WAYPOINT cw;
		cw.m_id = wo["id"].get<int>();
		cw.m_vPos.y = wo["lon"].get<double>();
		cw.m_vPos.x = wo["lat"].get<double>();
		cw.m_vPos.z = wo["alt"].get<double>();
		cw.m_vPos.w = wo["ground_alt"].get<double>();

		m_vWP.push_back(cw);
	}

	m_state = cetcus_receivedMission;
	LOG_I("Received flight data");
}

void _CETCUS::startFly(picojson::object& jo)
{
	int status = (int)jo["status"].get<double>();
	IF_(status != 0);

	m_flyID = jo["fly_id"].get<string>();

	m_state = cetcus_startFly;
	LOG_I("Start fly");
}

void _CETCUS::detailUpload(picojson::object& jo)
{
	int status = (int)jo["status"].get<double>();
	IF_(status != 0);

	LOG_I("Detail upload OK");
}

void _CETCUS::command(picojson::object& jo)
{
	picojson::object& d = jo["data"].get<object>();

	string msgID = d["message_id"].get<string>();
	CETCUS_CMD cmd = (CETCUS_CMD)d["cmd_name"].get<double>();

	//TODO:

	LOG_I("Received command");
}

void _CETCUS::endFly(picojson::object& jo)
{
	int status = (int)jo["status"].get<double>();
	IF_(status != 0);

	LOG_I("End fly OK");
}

void _CETCUS::planeConnect(void)
{
	IF_(check()<0);

	picojson::object o;
	o.insert(make_pair("action", value(string("plane_connect"))));
	o.insert(make_pair("uav_no", value(m_uavNo)));

	string msg = picojson::value(o).serialize() + m_msgFinishSend;
	m_pIO->write((unsigned char*)msg.c_str(), msg.size());
	LOG_I("Sent: " + msg);
}

void _CETCUS::startFly(void)
{
	IF_(check()<0);

	picojson::object o;
	o.insert(make_pair("action", value(string("start_fly"))));
	o.insert(make_pair("uav_no", value(m_uavNo)));
	o.insert(make_pair("time", value(tFormat())));
	o.insert(make_pair("airline_no", value(m_airlineNo)));
	o.insert(make_pair("start_point_name", value(m_wpStart.m_name)));
	o.insert(make_pair("start_point_lon", value(lf2str(m_wpStart.m_vPos.y,8))));
	o.insert(make_pair("start_point_lat", value(lf2str(m_wpStart.m_vPos.x,8))));
	o.insert(make_pair("start_point_alt", value(lf2str(m_wpStart.m_vPos.z,8))));

	picojson::array wpA;
	for(unsigned int i=0; i< m_vWP.size(); i++)
	{
		CETCUS_WAYPOINT* pWP = &m_vWP[i];
		object wp;
		wp.insert(make_pair("id", value(i2str(pWP->m_id))));
		wp.insert(make_pair("lon", value(lf2str(pWP->m_vPos.y,8))));
		wp.insert(make_pair("lat", value(lf2str(pWP->m_vPos.x,8))));
		wp.insert(make_pair("alt", value(lf2str(pWP->m_vPos.z,8))));
		wp.insert(make_pair("ground_alt", value(lf2str(pWP->m_vPos.w,8))));

		wpA.push_back(value(wp));
	}
	o.insert(make_pair("waypoints", value(wpA)));

	o.insert(make_pair("end_point_name", value(m_wpEnd.m_name)));
	o.insert(make_pair("end_point_lon", value(lf2str(m_wpEnd.m_vPos.y,8))));
	o.insert(make_pair("end_point_lat", value(lf2str(m_wpEnd.m_vPos.x,8))));
	o.insert(make_pair("end_point_alt", value(lf2str(m_wpEnd.m_vPos.z,8))));

	string msg = picojson::value(o).serialize() + m_msgFinishSend;
	m_pIO->write((unsigned char*)msg.c_str(), msg.size());
	LOG_I("Sent: " + msg);
}

void _CETCUS::detailUpload(void)
{
	IF_(check()<0);

	picojson::object o;
	o.insert(make_pair("action", value(string("detail_upload"))));

	picojson::object d;
	d.insert(make_pair("message", value(string("UAVGCS_YTC2_FLY_PARAM"))));
	d.insert(make_pair("fly_id", value(m_flyID)));
	d.insert(make_pair("uav_no", value(m_uavNo)));
	d.insert(make_pair("fly_status", value((double)m_flyStatus)));
	d.insert(make_pair("time", value(tFormat())));
	d.insert(make_pair("lon", value(m_vPos.y)));
	d.insert(make_pair("lat", value(m_vPos.x)));
	d.insert(make_pair("alt", value(m_vPos.z)));
	d.insert(make_pair("ground_alt", value(m_vPos.w)));
	d.insert(make_pair("course", value(m_course)));
	d.insert(make_pair("velocity_c", value(m_vSpeed.x)));
	d.insert(make_pair("pitch", value(m_vAtti.y)));
	d.insert(make_pair("roll", value(m_vAtti.z)));
	d.insert(make_pair("yaw", value(m_vAtti.x)));
	d.insert(make_pair("true_airspeed", value(m_vSpeed.y)));
	d.insert(make_pair("ground_speed", value(m_vSpeed.z)));
	d.insert(make_pair("remaining_oil", value((double)m_remainOil)));
	d.insert(make_pair("remaining_dis", value((double)m_remainDist)));
	d.insert(make_pair("remaining_time", value((double)m_remainTime)));
	d.insert(make_pair("mot_status", value((double)m_statusMot)));
	d.insert(make_pair("nav_status", value((double)m_statusNav)));
	d.insert(make_pair("com_status", value((double)m_statusCom)));

	o.insert(make_pair("data", value(d)));

	string msg = picojson::value(o).serialize() + m_msgFinishSend;
	m_pIO->write((unsigned char*)msg.c_str(), msg.size());
	LOG_I("Sent: " + msg);
}

void _CETCUS::endFly(void)
{
	IF_(check()<0);

	picojson::object o;
	o.insert(make_pair("action", value(string("end_fly"))));
	o.insert(make_pair("uav_no", value(m_uavNo)));
	o.insert(make_pair("time", value(tFormat())));
	o.insert(make_pair("fly_id", value(m_flyID)));

	string msg = picojson::value(o).serialize() + m_msgFinishSend;
	m_pIO->write((unsigned char*)msg.c_str(), msg.size());
	LOG_I("Sent: " + msg);
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

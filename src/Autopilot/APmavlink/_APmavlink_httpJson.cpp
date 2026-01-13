#include "_APmavlink_httpJson.h"

namespace kai
{

	_APmavlink_httpJson::_APmavlink_httpJson()
	{
		m_pAP = nullptr;
		m_url = "";
	}

	_APmavlink_httpJson::~_APmavlink_httpJson()
	{
	}

	bool _APmavlink_httpJson::init(const json &j)
	{
		IF_F(!this->_ModuleBase::init(j));

		m_url = j.value("url", "");

		IF_F(m_httpC.init());

		return true;
	}

	bool _APmavlink_httpJson::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		string n = j.value("_APmavlink_base", "");
		m_pAP = (_APmavlink_base *)(pM->findModule(n));
		NULL_F(m_pAP);

		return true;
	}

	bool _APmavlink_httpJson::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _APmavlink_httpJson::check(void)
	{
		NULL_F(m_pAP);
		NULL_F(m_pAP->getMavlink());

		return this->_ModuleBase::check();
	}

	void _APmavlink_httpJson::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateHttpSend();
		}
	}

	void _APmavlink_httpJson::updateHttpSend(void)
	{
		object o;
		// JO(o, "id", i2str(m_pDB->getID()));
		// JO(o, "lat", lf2str(vP.x, 10));
		// JO(o, "lng", lf2str(vP.y, 10));

		JO(o, "id", i2str(0));
		JO(o, "lat", lf2str(123.4567));
		JO(o, "lng", lf2str(765.4321));

		string jsonMsg = picojson::value(o).serialize();

		m_httpC.post_imageinfo(m_url.c_str(), jsonMsg.c_str());
	}

	void _APmavlink_httpJson::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		//		pC->addMsg("Local NED:");
	}

}

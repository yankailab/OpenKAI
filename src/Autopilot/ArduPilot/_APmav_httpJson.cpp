#include "_APmav_httpJson.h"

namespace kai
{

	_APmav_httpJson::_APmav_httpJson()
	{
	}

	_APmav_httpJson::~_APmav_httpJson()
	{
	}

	bool _APmav_httpJson::init(const json &j)
	{
		IF_F(!this->_ModuleBase::init(j));

		jKv(j, "url", m_url);

		IF_F(m_httpC.init());

		return true;
	}

	bool _APmav_httpJson::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		string n = "";
		jKv(j, "_APmav_base", n);
		m_pAP = (_APmav_base *)(pM->findModule(n));
		NULL_F(m_pAP);

		return true;
	}

	bool _APmav_httpJson::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _APmav_httpJson::check(void)
	{
		NULL_F(m_pAP);
		NULL_F(m_pAP->getMavlink());

		return this->_ModuleBase::check();
	}

	void _APmav_httpJson::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			updateHttpSend();
		}
	}

	void _APmav_httpJson::updateHttpSend(void)
	{
		//		object o;
		// JO(o, "id", i2str(m_pDB->getID()));
		// JO(o, "lat", lf2str(vP.x, 10));
		// JO(o, "lng", lf2str(vP.y, 10));

		// JO(o, "id", i2str(0));
		// JO(o, "lat", lf2str(123.4567));
		// JO(o, "lng", lf2str(765.4321));

		// string jsonMsg = picojson::value(o).serialize();

		// m_httpC.post_imageinfo(m_url.c_str(), jsonMsg.c_str());
	}

	void _APmav_httpJson::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		// _Console *pC = (_Console *)pConsole;
		//		pC->addMsg("Local NED:");
	}

}

#include "_MavAP_httpJson.h"

namespace kai
{

	_MavAP_httpJson::_MavAP_httpJson()
	{
		m_pAP = nullptr;
		m_url = "";

	}

	_MavAP_httpJson::~_MavAP_httpJson()
	{
	}

	int _MavAP_httpJson::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("url", &m_url);

		IF__(m_httpC.init(), OK_ERR_UNKNOWN);
   	
		return OK_OK;
	}

	int _MavAP_httpJson::link(void)
	{
		CHECK_(this->_ModuleBase::link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		n = "";
		pK->v("_MavAP_base", &n);
		m_pAP = (_MavAP_base *)(pK->findModule(n));
		NULL__(m_pAP, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	int _MavAP_httpJson::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _MavAP_httpJson::check(void)
	{
		NULL__(m_pAP, OK_ERR_NULLPTR);
		NULL__(m_pAP->m_pMav, OK_ERR_NULLPTR);

		return this->_ModuleBase::check();
	}

	void _MavAP_httpJson::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			updateHttpSend();

			m_pT->autoFPSto();
		}
	}

	void _MavAP_httpJson::updateHttpSend(void)
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

	void _MavAP_httpJson::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		//		pC->addMsg("Local NED:");
	}

}

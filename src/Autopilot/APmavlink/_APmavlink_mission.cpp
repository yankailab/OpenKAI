#include "_APmavlink_mission.h"

namespace kai
{

	_APmavlink_mission::_APmavlink_mission()
	{
		m_pAP = nullptr;
		m_mID = -1;
		m_mState = apMission_none;
	}

	_APmavlink_mission::~_APmavlink_mission()
	{
	}

	int _APmavlink_mission::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

//		pK->v("dS", &m_dS);

		return OK_OK;
	}

	int _APmavlink_mission::link(void)
	{
		CHECK_(this->_ModuleBase::link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		n = "";
		pK->v("_APmavlink_base", &n);
		m_pAP = (_APmavlink_base *)(pK->findModule(n));
		NULL__(m_pAP, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	int _APmavlink_mission::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _APmavlink_mission::check(void)
	{
		NULL__(m_pAP, OK_ERR_NULLPTR);
		NULL__(m_pAP->m_pMav, OK_ERR_NULLPTR);

		return this->_ModuleBase::check();
	}

	void _APmavlink_mission::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			updateMission();

			m_pT->autoFPSto();
		}
	}

	void _APmavlink_mission::updateMission(void)
	{
		IF_(check() != OK_OK);

		int apMode = m_pAP->getMode();
		vDouble4 vP = m_pAP->getGlobalPos();

		IF_(!m_pAP->bApArmed());
	}

	void _APmavlink_mission::send(void)
	{
		IF_(check() != OK_OK);

	}

	void _APmavlink_mission::clearMission(void)
	{
		m_vMission.clear();
		m_mID = -1;
	}

	void _APmavlink_mission::downloadMission(void)
	{

	}
	
	void _APmavlink_mission::uploadMission(void)
	{

	}
	
	void _APmavlink_mission::missionCount(void)
	{
		IF_(check() != OK_OK);

		mavlink_mission_count_t d;
		d.count = m_vMission.size();
//		d.mission_type = ;

		m_pAP->m_pMav->missionCount(d);
	}

	void _APmavlink_mission::missionRequestList(void)
	{
		IF_(check() != OK_OK);

		mavlink_mission_request_list_t d;
//		d.mission_type = ;
		m_pAP->m_pMav->missionRequestList(d);
	}

	void _APmavlink_mission::missionRequestInt(void)
	{
		IF_(check() != OK_OK);

		mavlink_mission_request_int_t d;
//		d.seq = ;
//		d.mission_type = ;
		m_pAP->m_pMav->missionRequestInt(d);
	}

	void _APmavlink_mission::missionItemInt(void)
	{
		IF_(check() != OK_OK);

		mavlink_mission_item_int_t d;
//		d.mission_type = ;
		m_pAP->m_pMav->missionItemInt(d);
	}

	void _APmavlink_mission::missionAck(void)
	{
		IF_(check() != OK_OK);

		mavlink_mission_ack_t d;
//		d.mission_type = ;
		m_pAP->m_pMav->missionAck(d);
	}

	void _APmavlink_mission::missionCurrent(void)
	{
		IF_(check() != OK_OK);

		mavlink_mission_current_t d;
//		d.mission_type = ;
		m_pAP->m_pMav->missionCurrent(d);
	}

	void _APmavlink_mission::missionSetCurrent(void)
	{
		IF_(check() != OK_OK);

		mavlink_mission_set_current_t d;
//		d.mission_type = ;
		m_pAP->m_pMav->missionSetCurrent(d);
	}

	void _APmavlink_mission::statusText(void)
	{
		IF_(check() != OK_OK);

		mavlink_statustext_t d;
//		d.mission_type = ;
		m_pAP->m_pMav->statusText(d);
	}

	void _APmavlink_mission::missionClearAll(void)
	{
		IF_(check() != OK_OK);

		mavlink_mission_clear_all_t d;
//		d.mission_type = ;
		m_pAP->m_pMav->missionClearAll(d);
	}

	void _APmavlink_mission::missionItemReached(void)
	{
		IF_(check() != OK_OK);

		mavlink_mission_item_reached_t d;
//		d.mission_type = ;
		m_pAP->m_pMav->missionItemReached(d);
	}

	void _APmavlink_mission::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("mID = " + i2str(m_mID), 1);
		pC->addMsg("nMission = " + i2str(m_vMission.size()), 1);
	}

}

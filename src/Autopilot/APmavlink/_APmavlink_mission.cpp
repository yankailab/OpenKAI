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
		
		_Mavlink* pMav = m_pAP->m_pMav;

		if(m_mState == apMission_none)
		{
			pMav->m_missionRequestInt.clearCbRecv(sCbMavRecvMissionRequestInt, this);
			pMav->m_missionAck.clearCbRecv(sCbMavRecvMissionAck, this);
			pMav->m_missionCount.clearCbRecv(sCbMavRecvMissionCount, this);

			return;
		}

		// Upload
		if(m_mState == apMission_UL_missionCount)
		{
			m_tOut.setTout(APMAV_MISSION_TOUT);
			m_tOut.start();

			m_mState = apMission_UL_missionRequestInt;
			pMav->m_missionRequestInt.addCbRecv(sCbMavRecvMissionRequestInt, this);
			pMav->m_missionAck.addCbRecv(sCbMavRecvMissionAck, this);

			sendMissionCount();
		}

		if(m_mState == apMission_UL_missionRequestInt)
		{
			IF_(!m_tOut.bTout(getTbootUs()));

			m_mState = apMission_UL_missionCount;
			return;
		}

		if(m_mState == apMission_UL_missionAck)
		{
			m_mState = apMission_none;
			return;			
		}

		// Download
		if(m_mState == apMission_DL_missionRequestList)
		{
			m_tOut.setTout(APMAV_MISSION_TOUT);
			m_tOut.start();

			m_mState = apMission_UL_missionCount;
			pMav->m_missionCount.addCbRecv(sCbMavRecvMissionRequestInt, this);

			sendMissionRequestList();
		}

		if(m_mState == apMission_DL_missionCount)
		{
			IF_(!m_tOut.bTout(getTbootUs()));

			m_mState = apMission_DL_missionRequestList;
			return;
		}

		if(m_mState == apMission_DL_missionRequestInt)
		{
			sendMissionRequestInt();			
		}

		if(m_mState == apMission_DL_missionAck)
		{
			sendMissionAck();			
		}

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
	
	void _APmavlink_mission::sendMissionCount(void)
	{
		mavlink_mission_count_t d;
		d.count = m_vMission.size();
//		d.mission_type = ;

		m_pAP->m_pMav->missionCount(d);
	}

	void _APmavlink_mission::CbMavRecvMissionRequestInt(void* pMsg)
	{
		IF_(check() != OK_OK);
		NULL_(pMsg);
		IF_(m_mState != apMission_UL_missionRequestInt);

		MavMissionRequestInt* pM = (MavMissionRequestInt*)pMsg;
		IF_(m_tOut.bTout(pM->m_tStamp));

		// restart timeout
		m_tOut.start();

		// reply the requested mission item
		mavlink_mission_item_int_t d;
//		d.seq = ;
//		d.mission_type = ;
		m_pAP->m_pMav->missionItemInt(d);
	}

	void _APmavlink_mission::CbMavRecvMissionAck(void* pMsg)
	{
		IF_(check() != OK_OK);
		NULL_(pMsg);
		IF_(m_mState != apMission_UL_missionRequestInt);

		MavMissionAck* pM = (MavMissionAck*)pMsg;
		IF_(m_tOut.bTout(pM->m_tStamp));

		// restart timeout
		m_tOut.start();

		m_mState = apMission_UL_missionAck;
	}

	void _APmavlink_mission::sendMissionRequestList(void)
	{
		mavlink_mission_request_list_t d;
//		d.mission_type = ;
		m_pAP->m_pMav->missionRequestList(d);
	}

	void _APmavlink_mission::CbMavRecvMissionCount(void* pMsg)
	{
		IF_(check() != OK_OK);
		NULL_(pMsg);
		IF_(m_mState != apMission_DL_missionCount);

		MavMissionCount* pM = (MavMissionCount*)pMsg;
		IF_(m_tOut.bTout(pM->m_tStamp));

		// restart timeout
		m_tOut.start();

		m_mState = apMission_DL_missionRequestInt;
	}

	void _APmavlink_mission::sendMissionRequestInt(void)
	{
		mavlink_mission_request_int_t d;
//		d.mission_type = ;
		m_pAP->m_pMav->missionRequestInt(d);
	}

	void _APmavlink_mission::CbMavRecvMissionItemInt(void* pMsg)
	{
		IF_(check() != OK_OK);
		NULL_(pMsg);
		IF_(m_mState != apMission_DL_missionRequestInt);

		MavMissionItemInt* pM = (MavMissionItemInt*)pMsg;
		IF_(m_tOut.bTout(pM->m_tStamp));

		// restart timeout
		m_tOut.start();

		IF_(pM->m_msg.seq < m_nMission-1);

		m_mState = apMission_DL_missionAck;
	}

	void _APmavlink_mission::sendMissionAck(void)
	{
		mavlink_mission_request_list_t d;
//		d.mission_type = ;
		m_pAP->m_pMav->missionRequestList(d);
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

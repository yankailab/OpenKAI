#include "_APmavlink_mission.h"

namespace kai
{

	_APmavlink_mission::_APmavlink_mission()
	{
		m_pAP = nullptr;
		m_mIdxDL = -1;
		m_mState = apMission_none;
		m_tOutSec = 10;
	}

	_APmavlink_mission::~_APmavlink_mission()
	{
	}

	int _APmavlink_mission::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("tOutSec", &m_tOutSec);

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

			downloadMission();
			updateMission();

			m_pT->autoFPSto();
		}
	}

	void _APmavlink_mission::updateMission(void)
	{
		IF_(check() != OK_OK);

		_Mavlink *pMav = m_pAP->m_pMav;

		if (m_mState == apMission_none)
		{
			pMav->m_missionRequestInt.clearCbRecv(sCbMavRecvMissionRequestInt, this);
			pMav->m_missionAck.clearCbRecv(sCbMavRecvMissionAck, this);
			pMav->m_missionCount.clearCbRecv(sCbMavRecvMissionCount, this);

			return;
		}

		// Upload
		if (m_mState == apMission_UL_missionCount)
		{
			m_tOut.setTout(m_tOutSec * USEC_1SEC);
			m_tOut.start();

			m_mState = apMission_UL_missionRequestInt;
			pMav->m_missionRequestInt.addCbRecv(sCbMavRecvMissionRequestInt, this);
			pMav->m_missionAck.addCbRecv(sCbMavRecvMissionAck, this);

			sendMissionCount();
		}

		if (m_mState == apMission_UL_missionRequestInt)
		{
			m_mState = apMission_UL_missionCount;
		}

		if (m_mState == apMission_UL_missionAck)
		{
			m_mState = apMission_none;
		}

		// Download
		if (m_mState == apMission_DL_missionRequestList)
		{
			m_tOut.setTout(m_tOutSec * USEC_1SEC);
			m_tOut.start();

			pMav->m_missionCount.addCbRecv(sCbMavRecvMissionCount, this);
			m_mState = apMission_DL_missionCount;
		}

		if (m_mState == apMission_DL_missionCount)
		{
			sendMissionRequestList();
			checkTimeOut();
		}

		if (m_mState == apMission_DL_missionRequestInt)
		{
			sendMissionRequestInt();
			checkTimeOut();
		}

		if (m_mState == apMission_DL_missionAck)
		{
			sendMissionAck();
			m_mState = apMission_none;
		}
	}

	void _APmavlink_mission::clearMission(void)
	{
		m_vMissionDL.clear();
		m_mIdxDL = -1;
	}

	// Upload
	void _APmavlink_mission::uploadMission(void)
	{
		// TODO:
	}

	void _APmavlink_mission::sendMissionCount(void)
	{
		mavlink_mission_count_t d;
		d.count = m_vMissionDL.size();

		m_pAP->m_pMav->missionCount(d);
	}

	void _APmavlink_mission::CbMavRecvMissionRequestInt(void *pMsg)
	{
		IF_(check() != OK_OK);
		NULL_(pMsg);
		IF_(m_mState != apMission_UL_missionRequestInt);

		MavMissionRequestInt *pM = (MavMissionRequestInt *)pMsg;

		// reply the requested mission item
		mavlink_mission_item_int_t d;
		//		d.seq = ;
		//		d.mission_type = ;
		m_pAP->m_pMav->missionItemInt(d);

		// restart timeout
		m_tOut.start();
	}

	void _APmavlink_mission::CbMavRecvMissionAck(void *pMsg)
	{
		IF_(check() != OK_OK);
		NULL_(pMsg);
		IF_(m_mState != apMission_UL_missionRequestInt);

		MavMissionAck *pM = (MavMissionAck *)pMsg;

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

	// Download
	void _APmavlink_mission::downloadMission(void)
	{
		IF_(m_mState == apMission_DL_missionRequestList);
		IF_(m_mState == apMission_DL_missionCount);
		IF_(m_mState == apMission_DL_missionRequestInt);
		IF_(m_mState == apMission_DL_missionAck);

		m_mState = apMission_DL_missionRequestList;
	}

	void _APmavlink_mission::CbMavRecvMissionCount(void *pMsg)
	{
		IF_(check() != OK_OK);
		NULL_(pMsg);
		IF_(m_mState != apMission_DL_missionCount);

		MavMissionCount *pM = (MavMissionCount *)pMsg;
		m_nMissionDL = pM->m_msg.count;
		m_iMissionDL = 0;

		m_pAP->m_pMav->m_missionItemInt.addCbRecv(sCbMavRecvMissionItemInt, this);
		m_mState = apMission_DL_missionRequestInt;
		m_tOut.start();
	}

	void _APmavlink_mission::sendMissionRequestInt(void)
	{
		mavlink_mission_request_int_t d;
		d.mission_type = MAV_MISSION_TYPE_MISSION;
		d.seq = m_iMissionDL;
		m_pAP->m_pMav->missionRequestInt(d);
	}

	void _APmavlink_mission::CbMavRecvMissionItemInt(void *pMsg)
	{
		IF_(check() != OK_OK);
		NULL_(pMsg);
		IF_(m_mState != apMission_DL_missionRequestInt);

		MavMissionItemInt *pM = (MavMissionItemInt *)pMsg;
		IF_(pM->m_msg.seq < m_iMissionDL);

		AP_MISSION m;
		m.m_missionType = pM->m_msg.mission_type;
		// TODO:

		m_iMissionDL++;

		m_tOut.start();
		IF_(pM->m_msg.seq < m_nMissionDL - 1);

		m_mState = apMission_DL_missionAck;
	}

	void _APmavlink_mission::sendMissionAck(void)
	{
		mavlink_mission_ack_t d;
		m_pAP->m_pMav->missionAck(d);
	}

	void _APmavlink_mission::checkTimeOut(void)
	{
		if (m_tOut.bTout(getTbootUs()))
			m_mState = apMission_none;
	}

	void _APmavlink_mission::missionCurrent(void)
	{
		IF_(check() != OK_OK);

		mavlink_mission_current_t d;
		m_pAP->m_pMav->missionCurrent(d);
	}

	void _APmavlink_mission::missionSetCurrent(void)
	{
		IF_(check() != OK_OK);

		mavlink_mission_set_current_t d;
		m_pAP->m_pMav->missionSetCurrent(d);
	}

	void _APmavlink_mission::statusText(void)
	{
		IF_(check() != OK_OK);

		mavlink_statustext_t d;
		m_pAP->m_pMav->statusText(d);
	}

	void _APmavlink_mission::missionClearAll(void)
	{
		IF_(check() != OK_OK);

		mavlink_mission_clear_all_t d;
		m_pAP->m_pMav->missionClearAll(d);
	}

	void _APmavlink_mission::missionItemReached(void)
	{
		IF_(check() != OK_OK);

		mavlink_mission_item_reached_t d;
		m_pAP->m_pMav->missionItemReached(d);
	}

	void _APmavlink_mission::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("mState = " + i2str(m_mState), 1);

		pC->addMsg("Upload------", 1);
		pC->addMsg("iMissionUL = " + i2str(m_vMissionUL.size()), 1);

		pC->addMsg("Download----", 1);
		pC->addMsg("mIdxDL = " + i2str(m_mIdxDL), 1);
		pC->addMsg("nMissionDL = " + i2str(m_nMissionDL), 1);
		pC->addMsg("iMissionDL = " + i2str(m_iMissionDL), 1);
	}

}

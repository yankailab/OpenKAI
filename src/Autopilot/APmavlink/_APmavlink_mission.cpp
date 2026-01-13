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

	bool _APmavlink_mission::init(const json &j)
	{
		IF_F(!this->_ModuleBase::init(j));

		m_tOutSec = j.value("tOutSec", 10);

		return true;
	}

	bool _APmavlink_mission::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		string n = j.value("_APmavlink_base", "");
		m_pAP = (_APmavlink_base *)(pM->findModule(n));
		NULL_F(m_pAP);

		return true;
	}

	bool _APmavlink_mission::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _APmavlink_mission::check(void)
	{
		NULL_F(m_pAP);
		NULL_F(m_pAP->getMavlink());

		return this->_ModuleBase::check();
	}

	void _APmavlink_mission::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			downloadMission();
			updateMission();
		}
	}

	void _APmavlink_mission::updateMission(void)
	{
		IF_(!check());

		_Mavlink *pMav = m_pAP->getMavlink();

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
			m_tOut.reStart();

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
			m_tOut.reStart();

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

		m_pAP->getMavlink()->missionCount(d);
	}

	void _APmavlink_mission::CbMavRecvMissionRequestInt(void *pMsg)
	{
		IF_(!check());
		NULL_(pMsg);
		IF_(m_mState != apMission_UL_missionRequestInt);

		MavMissionRequestInt *pM = (MavMissionRequestInt *)pMsg;

		// reply the requested mission item
		mavlink_mission_item_int_t d;
		//		d.seq = ;
		//		d.mission_type = ;
		m_pAP->getMavlink()->missionItemInt(d);

		// restart timeout
		m_tOut.reStart();
	}

	void _APmavlink_mission::CbMavRecvMissionAck(void *pMsg)
	{
		IF_(!check());
		NULL_(pMsg);
		IF_(m_mState != apMission_UL_missionRequestInt);

		MavMissionAck *pM = (MavMissionAck *)pMsg;

		// restart timeout
		m_tOut.reStart();

		m_mState = apMission_UL_missionAck;
	}

	void _APmavlink_mission::sendMissionRequestList(void)
	{
		mavlink_mission_request_list_t d;
		//		d.mission_type = ;
		m_pAP->getMavlink()->missionRequestList(d);
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
		IF_(!check());
		NULL_(pMsg);
		IF_(m_mState != apMission_DL_missionCount);

		MavMissionCount *pM = (MavMissionCount *)pMsg;
		m_nMissionDL = pM->m_msg.count;
		m_iMissionDL = 0;

		m_pAP->getMavlink()->m_missionItemInt.addCbRecv(sCbMavRecvMissionItemInt, this);
		m_mState = apMission_DL_missionRequestInt;
		m_tOut.reStart();
	}

	void _APmavlink_mission::sendMissionRequestInt(void)
	{
		mavlink_mission_request_int_t d;
		d.mission_type = MAV_MISSION_TYPE_MISSION;
		d.seq = m_iMissionDL;
		m_pAP->getMavlink()->missionRequestInt(d);
	}

	void _APmavlink_mission::CbMavRecvMissionItemInt(void *pMsg)
	{
		IF_(!check());
		NULL_(pMsg);
		IF_(m_mState != apMission_DL_missionRequestInt);

		MavMissionItemInt *pM = (MavMissionItemInt *)pMsg;
		IF_(pM->m_msg.seq < m_iMissionDL);

		AP_MISSION m;
		m.m_missionType = pM->m_msg.mission_type;
		// TODO:

		m_iMissionDL++;

		m_tOut.reStart();
		IF_(pM->m_msg.seq < m_nMissionDL - 1);

		m_mState = apMission_DL_missionAck;
	}

	void _APmavlink_mission::sendMissionAck(void)
	{
		mavlink_mission_ack_t d;
		m_pAP->getMavlink()->missionAck(d);
	}

	void _APmavlink_mission::checkTimeOut(void)
	{
		if (m_tOut.bTout(getTbootUs()))
			m_mState = apMission_none;
	}

	void _APmavlink_mission::missionCurrent(void)
	{
		IF_(!check());

		mavlink_mission_current_t d;
		m_pAP->getMavlink()->missionCurrent(d);
	}

	void _APmavlink_mission::missionSetCurrent(void)
	{
		IF_(!check());

		mavlink_mission_set_current_t d;
		m_pAP->getMavlink()->missionSetCurrent(d);
	}

	void _APmavlink_mission::statusText(void)
	{
		IF_(!check());

		mavlink_statustext_t d;
		m_pAP->getMavlink()->statusText(d);
	}

	void _APmavlink_mission::missionClearAll(void)
	{
		IF_(!check());

		mavlink_mission_clear_all_t d;
		m_pAP->getMavlink()->missionClearAll(d);
	}

	void _APmavlink_mission::missionItemReached(void)
	{
		IF_(!check());

		mavlink_mission_item_reached_t d;
		m_pAP->getMavlink()->missionItemReached(d);
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

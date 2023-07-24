/*
 * _SwarmBase.cpp
 *
 *  Created on: April 3, 2023
 *      Author: yankai
 */

#include "_SwarmBase.h"

namespace kai
{

	_SwarmBase::_SwarmBase()
	{
		m_tExpire = UINT64_MAX;
	}

	_SwarmBase::~_SwarmBase()
	{
	}

	bool _SwarmBase::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("tExpire", &m_tExpire);

		return true;
	}

	bool _SwarmBase::link(void)
	{
		IF_F(!this->_ModuleBase::link());

		return true;
	}

	int _SwarmBase::check(void)
	{
		return this->_ModuleBase::check();
	}

	void _SwarmBase::handleMsgHB(const SWMSG_HB& m)
	{
		SWARM_NODE* pN = getNode(m.m_srcID);
		if(!pN)
		{
			SWARM_NODE n;
			pN = addNode(n);
		}

		pN->m_srcNetAddr = m.m_srcNetAddr;
		pN->m_id = m.m_srcID;
		pN->m_vPos.x = ((double)m.m_lat) * 1e-7;
		pN->m_vPos.y = ((double)m.m_lng) * 1e-7;
		pN->m_alt = ((float)m.m_alt) * 1e-2;
		pN->m_spd = ((float)m.m_spd) * 1e-2;
		pN->m_batt = ((float)m.m_batt);
		pN->m_iMsg = m.m_iMsg;
		pN->m_tLastUpdate = getApproxTbootUs();
	}

    vector<SWARM_NODE>* _SwarmBase::getSwarmNode(void)
    {
        return &m_vNodes;
    }

	int _SwarmBase::getNodeIdx(const uint64_t &ID)
	{
		int n = m_vNodes.size();
		IF__(n <= 0, -1);

		for (int i = 0; i < n; i++)
		{
			IF_CONT(m_vNodes[i].m_id != ID);
			return i;
		}

		return -1;
	}

	SWARM_NODE *_SwarmBase::getNode(const uint64_t &ID)
	{
		int i = getNodeIdx(ID);
		IF_N(i < 0);

		return &m_vNodes[i];
	}

	SWARM_NODE* _SwarmBase::addNode(const SWARM_NODE &n)
	{
		int i = getNodeIdx(n.m_id);
		if(i >= 0)
			return &m_vNodes[i];

		m_vNodes.push_back(n);
		return &m_vNodes[m_vNodes.size()];
	}

	bool _SwarmBase::updateNode(const SWARM_NODE &n)
	{
		int i = getNodeIdx(n.m_id);
		IF_F(i < 0);

		m_vNodes[i] = n;
		return true;
	}

	void _SwarmBase::deleteNode(const uint64_t &ID)
	{
		int i = getNodeIdx(ID);
		IF_(i < 0);

		deleteNode(i);
	}

	void _SwarmBase::deleteNode(int i)
	{
		IF_(i < 0);
		IF_(i >= m_vNodes.size());

		m_vNodes.erase(m_vNodes.begin() + i);
	}

	void _SwarmBase::deleteExpiredNodes(const uint64_t &tExpire)
	{
		uint64_t tNow = getApproxTbootUs();
		for (int i = 0; i < m_vNodes.size(); i++)
		{
			IF_CONT(tNow - m_vNodes[i].m_tLastUpdate < tExpire);

			deleteNode(i);
		}

		m_vNodes.shrink_to_fit();
	}

	SWARM_NODE *_SwarmBase::findClosestNode(vDouble2 vPos)
	{
		IF_N(m_vNodes.size() <= 0);

		int iN = -1;
		double dMin = DBL_MAX;
		for (int i = 0; i < m_vNodes.size(); i++)
		{
			SWARM_NODE *pN = &m_vNodes[i];
			double d = (pN->m_vPos - vPos).len();
			IF_CONT(d > dMin);

			iN = i;
		}

		IF_N(iN < 0);
		return &m_vNodes[iN];
	}

	int _SwarmBase::getNodesWithinRadius(const vDouble2 &vPos, float r)
	{
		return 0;
	}

	void _SwarmBase::console(void *pConsole)
	{
		NULL_(pConsole);
		IF_(check() < 0);
		this->_ModuleBase::console(pConsole);

		// string msg = "id=" + i2str(pO->getTopClass());
		// ((_Console *)pConsole)->addMsg(msg, 1);
	}

}

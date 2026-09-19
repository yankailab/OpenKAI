#include "_OctreeBase.h"

namespace kai
{

	_OctreeBase::_OctreeBase()
	{
	}

	_OctreeBase::~_OctreeBase()
	{
	}

	bool _OctreeBase::init(const json &j)
	{
		IF_F(!this->_ReferenceFrame::init(j));

		return true;
	}

	bool _OctreeBase::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ReferenceFrame::link(j, pM));

		return true;
	}

	bool _OctreeBase::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _OctreeBase::check(void)
	{
		return this->_ReferenceFrame::check();
	}

	void _OctreeBase::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			updateOctree();
		}
	}

	void _OctreeBase::updateOctree(void)
	{
		IF_(!check());
	}

}

#include "_OctreeBase.h"

namespace kai
{

	_OctreeBase::_OctreeBase()
	{
	}

	_OctreeBase::~_OctreeBase()
	{
	}

	bool _OctreeBase::loadConfig(void)
	{
		IF_F(!this->_ReferenceFrame::loadConfig());

		return true;
	}

	bool _OctreeBase::saveConfig(bool bExport)
	{
		IF_F(!_ReferenceFrame::saveConfig(false));

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _OctreeBase::link(void)
	{
		IF_F(!this->_ReferenceFrame::link());

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

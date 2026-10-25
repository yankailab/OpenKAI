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
		IF_F(!this->_ModuleBase::loadConfig());

		return true;
	}

	bool _OctreeBase::saveConfig(bool bExport)
	{
		IF_F(!_ModuleBase::saveConfig(false));

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _OctreeBase::link(InstanceMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(pM));

		string tfName;
		jKv(*m_pJ, "_TransformRigid", tfName);
		m_pTf = dynamic_cast<_TransformRigid *>(static_cast<_ModuleBase *>(pM->findModule(tfName)));
		IF_Le_F(!tfName.empty() && !m_pTf, "_TransformRigid not found: " + tfName);

		return true;
	}

	bool _OctreeBase::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _OctreeBase::check(void)
	{
		return this->_ModuleBase::check();
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

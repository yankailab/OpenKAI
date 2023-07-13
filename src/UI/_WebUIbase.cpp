/*
 * Window.cpp
 *
 *  Created on: July 10, 2023
 *      Author: Kai Yan
 */

#include "_WebUIbase.h"

namespace kai
{

	_WebUIbase::_WebUIbase()
	{
		m_pTui = NULL;
		m_rootDir = "";
		m_fHtml = "<html>Hello World!</html>";
		m_wd = 0;
		m_bMultiAccess = false;
	}

	_WebUIbase::~_WebUIbase()
	{
		DEL(m_pTui);
	}

	bool _WebUIbase::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("rootDir", &m_rootDir);
		pK->v("fHtml", &m_fHtml);
		pK->v("bMultiAccess", &m_bMultiAccess);

		Kiss *pKt = pK->child("threadUI");
		IF_F(pKt->empty());
		m_pTui = new _Thread();
		if (!m_pTui->init(pKt))
		{
			DEL(m_pTui);
			return false;
		}
		pKt->m_pInst = m_pTui;

		return true;
	}

	bool _WebUIbase::link(void)
	{
		IF_F(!this->_ModuleBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		IF_F(!m_pTui->link());

		vector<string> vB;
		pK->a("vBASE", &vB);
		for (string p : vB)
		{
			BASE *pB = (BASE *)(pK->getInst(p));
			IF_CONT(!pB);

			m_vpB.push_back(pB);
		}

		return true;
	}

	bool _WebUIbase::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _WebUIbase::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			show();

			m_pT->autoFPSto();
		}
	}

	void _WebUIbase::show(void)
	{
		m_wd = webui_new_window();
		webui_set_root_folder(m_wd, m_rootDir.c_str());
		webui_bind(m_wd, "myID", cbEvent);
		webui_set_multi_access(m_wd, m_bMultiAccess);
		webui_show(m_wd, m_fHtml.c_str());
		webui_wait();
//		webui_destroy(m_wd);

		m_wd = 0;
	}

}

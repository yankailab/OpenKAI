#include "ActionBase.h"

namespace kai
{

ActionBase::ActionBase()
{
	m_pVI = NULL;
	m_pMavlink = NULL;
	m_pCtrl = NULL;

}

ActionBase::~ActionBase()
{
}

bool ActionBase::init(Config* pConfig, string* pName)
{
	if (this->BASE::init(pConfig,pName)==false)
		return false;

	Config* pC = pConfig->o(*pName);

	return true;
}

void ActionBase::update(void)
{

}


bool ActionBase::draw(Frame* pFrame, iVec4* pTextPos)
{
	if (pFrame == NULL)
		return false;

	return true;
}


}

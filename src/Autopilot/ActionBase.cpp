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

bool ActionBase::init(Config* pConfig, string name)
{
	if(pConfig==NULL)return false;
	if(name.empty())return false;

	Config* pC = pConfig->o(name);
	if(pC->empty())return false;


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

#include "ActionBase.h"

namespace kai
{

ActionBase::ActionBase()
{
	m_pCtrl = NULL;

}

ActionBase::~ActionBase()
{
}

bool ActionBase::init(Config* pConfig, AUTOPILOT_CONTROL* pAC)
{
	if (this->BASE::init(pConfig)==false)
		return false;

	NULL_F(pAC);

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

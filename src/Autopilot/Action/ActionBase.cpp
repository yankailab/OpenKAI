#include "ActionBase.h"

namespace kai
{

ActionBase::ActionBase()
{
}

ActionBase::~ActionBase()
{
}

bool ActionBase::init(Config* pConfig)
{
	if (this->BASE::init(pConfig)==false)
		return false;

	return true;
}

void ActionBase::update(void)
{

}

}

#include "ActionBase.h"

namespace kai
{

ActionBase::ActionBase()
{
}

ActionBase::~ActionBase()
{
}

bool ActionBase::init(Kiss* pKiss)
{
	if (this->BASE::init(pKiss)==false)
		return false;

	return true;
}

void ActionBase::update(void)
{

}

}

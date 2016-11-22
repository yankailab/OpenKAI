#include "ActionBase.h"

namespace kai
{

ActionBase::ActionBase()
{
}

ActionBase::~ActionBase()
{
}

bool ActionBase::init(void* pKiss)
{
	CHECK_F(this->BASE::init(pKiss)==false);

	return true;
}

bool ActionBase::link(void)
{
	return true;
}

void ActionBase::update(void)
{

}

bool ActionBase::draw(Frame* pFrame, vInt4* pTextPos)
{
	return true;
}


}

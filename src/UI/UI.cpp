/*
 * UI.cpp
 *
 *  Created on: Aug 23, 2015
 *      Author: yankai
 */

#include "UI.h"

namespace kai
{

UI::UI()
{

}

UI::~UI()
{
	// TODO Auto-generated destructor stub
}

bool UI::init(Config* pConfig, string name)
{
	if(pConfig==NULL)return false;
	if(name.empty())return false;


	return true;
}


} /* namespace kai */

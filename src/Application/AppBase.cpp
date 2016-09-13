/*
 * ApplicationBase.cpp
 *
 *  Created on: Aug 22, 2016
 *      Author: root
 */

#include "AppBase.h"

namespace kai
{

AppBase::AppBase()
{
	// TODO Auto-generated constructor stub
	m_name = "";
	m_bShowScreen = 1;
	m_bFullScreen = 0;
	m_waitKey = 50;
	m_bRun = true;
	m_key = 0;


}

AppBase::~AppBase()
{
	// TODO Auto-generated destructor stub
}

}

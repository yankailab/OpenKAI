/*
 * AppBase.h
 *
 *  Created on: Aug 22, 2016
 *      Author: Kai Yan
 */

#ifndef OPENKAI_SRC_APPLICATION_APPBASE_H_
#define OPENKAI_SRC_APPLICATION_APPBASE_H_

#include "../Base/common.h"

namespace kai
{

class AppBase
{
public:
	AppBase();
	virtual ~AppBase();

public:
	string m_name;
	int m_key;
	bool m_bRun;

	int m_bShowScreen;
	int m_bFullScreen;
	int m_waitKey;
};

}

#endif /* OPENKAI_SRC_APPLICATION_APPBASE_H_ */

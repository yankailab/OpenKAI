/*
 * ApplicationBase.h
 *
 *  Created on: Aug 22, 2016
 *      Author: root
 */

#ifndef OPENKAI_SRC_APPLICATION_APPBASE_H_
#define OPENKAI_SRC_APPLICATION_APPBASE_H_

namespace kai
{

class AppBase
{
public:
	AppBase();
	virtual ~AppBase();

public:
	int m_key;
	bool m_bRun;

	int m_bShowScreen;
	int m_bFullScreen;
	int m_waitKey;
};

}

#endif /* OPENKAI_SRC_APPLICATION_APPBASE_H_ */
